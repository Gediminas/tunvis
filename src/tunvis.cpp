#include "tools/IPv4.h"
#include "tools/Log.h"
#include "tools/Routing.h"
#include "tools/Rules.h"
#include "tools/Tracking.h"
#include "tools/tun.h"

#include <unistd.h> //sleep

constexpr int32_t     c_nBufferSize = 2000; // for tun/tap must be >= 1500
constexpr const char *c_sTunName1   = "tunvis1";
constexpr const char *c_sTunName2   = "tunvis2";

std::string g_sEthName;

void signal_callback_handler(int signum) {
   std::cout << "Program terminating " << signum << std::endl;
   routing::DestroyTunnelRoutes(g_sEthName.c_str(), c_sTunName1, c_sTunName2);
   // sleep(3);
   exit(signum);
}

int main() {
    PrintAppTitle();

    signal(SIGINT, signal_callback_handler);

    g_sEthName = routing::GetDefaultEthName();
    const int fdTun1 = tun::InitTun(c_sTunName1);
    const int fdTun2 = tun::InitTun(c_sTunName2);

    std::cout << "Successfully connected to interfaces " << c_sTunName1 << " & " << c_sTunName2 << std::endl;

    routing::DestroyTunnelRoutes(g_sEthName.c_str(), c_sTunName1, c_sTunName2); // just in case
    routing::CreateTunnelRoutes(g_sEthName.c_str(), c_sTunName1, c_sTunName2);
    PrintTunnel(g_sEthName.c_str(), c_sTunName1, c_sTunName2);

    const std::vector<CFilterRule> arRules = filter_rules::readRules("dat/rules1.txt");
    PrintRules(arRules);

    sleep(1000);
    return 0;

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
            bool bTerminate = false;
            const uint16_t    uRead = tun::Read(fdTun1, buffer, sizeof(buffer));
            const CIpv4Packet packet = ipv4::ParseIpv4Packet(buffer);
            const int32_t     nRule  = filter_rules::findLastRule(arRules, packet.uDst, packet.eProtocol);

            if (nRule != -1) {
                const CFilterRule &rule = arRules[nRule];
                CRuleTrack &track = arTrack[nRule];
                bTerminate = track.bTerminate;

                PrintCurrentDateTime();
                PrintTraffic(nPacketCounter, uRead, packet, bTerminate, false);
                PrintAppliedRule(rule, false);
                PrintTrackingDetails(rule, track, 0, false);
                std::cout << std::endl;
            }

            if (!bTerminate) {
                tun::Write(fdTun2, buffer, uRead);
            }
        }

        if (FD_ISSET(fdTun2, &fdSet)) {
            ++nPacketCounter;
            bool bTerminate = false;
            const uint16_t uRead = tun::Read(fdTun2, buffer, sizeof(buffer));
            const CIpv4Packet    packet  = ipv4::ParseIpv4Packet(buffer);
            std::time_t    now   = std::time(nullptr);

            const int32_t nRule = filter_rules::findLastRule(arRules, packet.uSrc, packet.eProtocol);
            if (nRule != -1) {
                PrintCurrentDateTime();

                const CFilterRule &rule = arRules[nRule];
                CRuleTrack &track = arTrack[nRule];

                bTerminate = CheckRuleForTerm(rule, packet, track, uRead);
                track.bTerminate = bTerminate;

                PrintTraffic(nPacketCounter, uRead, packet, bTerminate, true);
                PrintAppliedRule(rule, true);
                PrintTrackingDetails(rule, track, now, true);
                std::cout << std::endl;
            }

            if (!bTerminate) {
                tun::Write(fdTun1, buffer, uRead);
            }
        }
    }

    return 0;
}
