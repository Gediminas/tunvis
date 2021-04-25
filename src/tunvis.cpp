#include "tools/IPv4.h"
#include "tools/Log.h"
#include "tools/Routing.h"
#include "tools/Rules.h"
#include "tools/Tracking.h"
#include "tools/Tun.h"

constexpr int32_t     c_nBufferSize = 2000; // for tun/tap must be >= 1500
constexpr const char *c_sTunName1   = "tunvis1";
constexpr const char *c_sTunName2   = "tunvis2";

std::string g_sEthName;
std::string g_sEthIP;

namespace internal {
    static void OnTerminate(int signum) {
        std::cout << std::endl;
        std::cout << "\033[1;33m" << "Terminating..." << "\033[0m" << std::endl;
        std::cout << "\033[32m"   << "Destroying tunnel" << "\033[0m" << std::endl;

        routing::DestroyTunnelRoutes(c_sTunName1, c_sTunName2, g_sEthName.c_str(), g_sEthIP.c_str());

        std::cout << "\033[1;33m" << "Terminated" << "\033[0m" << std::endl;
        exit(signum);
    }

    static inline std::tuple<std::string, std::string> ParseAppParams(int argc, char* argv[]) {
        std::string sRulesFile, sInterface;
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if ((arg == "-h") || (arg == "--help")) {
                PrintUsage(argv[0]);
                exit(-1);
            } else if ((arg == "-i") || (arg == "--interface")) {
                if (++i < argc) {
                    sInterface = argv[i];
                } else {
                    std::cerr << "-i / --interface option requires one argument." << std::endl;
                    exit(-1);
                }
            } else {
                sRulesFile = argv[i];
            }
        }
        if (sRulesFile.empty()) {
            PrintUsage(argv[0]);
            exit(-1);
        }
        return std::make_tuple(sRulesFile, sInterface);
    }
}

int main(int argc, char* argv[]) {
    auto [sRulesFile, sInterface] = internal::ParseAppParams(argc, argv);

    signal(SIGINT, internal::OnTerminate);

    PrintAppTitle();

    g_sEthName = sInterface.empty() ? routing::GetDefaultEthName() : sInterface;
    g_sEthIP   = routing::GetIPByDev(g_sEthName.c_str());

    std::cout << "\033[32m" << "Network interface used: " << g_sEthName << " (" << g_sEthIP << ")" << "\033[0m" << std::endl;

    std::cout << "\033[32m" << "Creating TUN interfaces " << c_sTunName1 << " & " << c_sTunName2 << "..." << "\033[0m" << std::endl;
    const int fdTun1 = tun::InitTun(c_sTunName1);
    const int fdTun2 = tun::InitTun(c_sTunName2);
    std::cout << "\033[32m" << "Successfully connected to interfaces " << c_sTunName1 << " & " << c_sTunName2 << " \033[0m" << std::endl;

    std::cout << "\033[32m" << "Destroying tunnel (if exists)" << "\033[0m" << std::endl;
    routing::DestroyTunnelRoutes(c_sTunName1, c_sTunName2, g_sEthName.c_str(), g_sEthIP.c_str()); // just in case

    std::cout << "\033[32m" << "Creating tunnel" << "\033[0m" << std::endl;
    routing::CreateTunnelRoutes(c_sTunName1, c_sTunName2, g_sEthName.c_str(), g_sEthIP.c_str());

    PrintTunnel(g_sEthName.c_str(), c_sTunName1, c_sTunName2);

    std::cout << "\033[32m" << "Loading rules from " << sRulesFile << "..."<< "\033[0m" << std::endl;
    const std::vector<CRule> arRules = filter_rules::readRules(sRulesFile.c_str());
    PrintRules(arRules);

    std::vector<CRuleTrack> arTrack(arRules.size());

    char buffer[c_nBufferSize];
    const int maxfd = (fdTun1 > fdTun2) ? fdTun1 : fdTun2; //use select() to handle two descriptors at once
    int64_t nPacketCounter = 0;

    while(1) {
        fd_set fdSet;
        FD_ZERO(&fdSet);
        FD_SET(fdTun1, &fdSet);
        FD_SET(fdTun2, &fdSet);

        const int nRet = select(maxfd + 1, &fdSet, NULL, NULL, NULL);
        if (nRet < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("select()");
            exit(1);
        }

        if (FD_ISSET(fdTun1, &fdSet)) {
            ++nPacketCounter;
            bool bAccept = true;
            const uint16_t    uRead = tun::Read(fdTun1, buffer, sizeof(buffer));
            const CIpv4Packet packet = ipv4::ParseIpv4PacketHeader(buffer, uRead);
            const int32_t     nRule  = filter_rules::findLastRule(arRules, packet.uDst, packet.eProtocol);
            if (nRule != -1) {
                const CRule &rule = arRules[nRule];
                CRuleTrack &track = arTrack[nRule];
                bAccept = track.bAccept;

                PrintCurrentDateTime();
                PrintTraffic(nPacketCounter, uRead, packet, track.bAccept, false);
                PrintAppliedRule(rule, false);
                PrintTrackingDetails(rule, track, false);
                std::cout << std::endl;
            }
            if (bAccept) {
                tun::Write(fdTun2, buffer, uRead);
            }
        }

        if (FD_ISSET(fdTun2, &fdSet)) {
            ++nPacketCounter;
            bool bAccept = true;
            const uint16_t uRead = tun::Read(fdTun2, buffer, sizeof(buffer));
            const CIpv4Packet    packet  = ipv4::ParseIpv4PacketHeader(buffer, uRead);
            const int32_t nRule = filter_rules::findLastRule(arRules, packet.uSrc, packet.eProtocol);
            if (nRule != -1) {
                const CRule &rule = arRules[nRule];
                CRuleTrack &track = arTrack[nRule];

                UpdateTracking(rule, packet, track, buffer, uRead);
                bAccept = track.bAccept;

                PrintCurrentDateTime();
                PrintTraffic(nPacketCounter, uRead, packet, track.bAccept, true);
                PrintAppliedRule(rule, true);
                PrintTrackingDetails(rule, track, true);
                std::cout << std::endl;
            }
            if (bAccept) {
                tun::Write(fdTun1, buffer, uRead);
            }
        }
    }

    return 0;
}
