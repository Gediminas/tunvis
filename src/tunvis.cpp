#include <iostream>

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/time.h>
#include <linux/if_tun.h>
// #include <stdlib.h>
// #include <string.h>
// #include <net/if.h>
// #include <sys/socket.h>
// #include <sys/ioctl.h>
// #include <sys/stat.h>
// #include <arpa/inet.h>
// #include <sys/select.h>
// #include <errno.h>
// #include <stdarg.h>

#include "routing.h"
#include "utils/tun.h"
#include "utils/ipv4_util.h"
#include "utils/filter_rules.h"

constexpr int32_t     c_nBufferSize = 2000; // for tun/tap must be >= 1500
constexpr const char *c_sEthName    = "enp0s3";
constexpr const char *c_sTunName1   = "tunvis1";
constexpr const char *c_sTunName2   = "tunvis2";

void print_current_time() {
    std::time_t t = std::time(nullptr);
    char sTime[100];
    if (std::strftime(sTime, sizeof(sTime), "%Y-%m-%d %H:%M:%S", std::localtime(&t))) {
        std::cout << "\033[90m" << sTime << "\033[0m";
    }
}

void PrintOutgoingPacket(int64_t nPacketCounter, int16_t uRead, const CInfo &info) {
    std::cout << "\033[92m";
    std::cout << " " << nPacketCounter <<  ": " << uRead << " B";
    std::cout << " ----> " << ipv4::numberToAddress(info.uDst);
    std::cout << " " << info.sProtocol;
    std::cout << "\033[0m";
}

void PrintIncommingPacket(int64_t nPacketCounter, int16_t uRead, const CInfo &info, bool bTerminate) {
    std::cout << "\033[32m" << " " << nPacketCounter <<  ": " << "\033[32m";
    std::cout << (bTerminate ? "\033[91m" : "\033[32m");
    std::cout << uRead << " B";
    std::cout << (bTerminate ? " <-x--" : " <---- ");
    std::cout << "\033[0m";

    std::cout << "\033[32m";
    std::cout << ipv4::numberToAddress(info.uSrc);
    std::cout << "\033[0m";
    std::cout << " " << info.sProtocol;
}

void signal_callback_handler(int signum) {
   std::cout << "Program terminating " << signum << std::endl;
   DestroyTunnelRoutes(c_sEthName, c_sTunName1, c_sTunName2);
   sleep(3);
   exit(signum);
}

int main() {
    std::cout << "\033[1;33m" << "=======================================================" << "\033[0m" << std::endl;
    std::cout << "\033[1;33m" << "***                  Tunnel Vission                 ***" << "\033[0m" << std::endl;
    std::cout << "\033[1;33m" << "=======================================================" << "\033[0m" << std::endl;

    signal(SIGINT, signal_callback_handler);

    const int fdTun1 = tun::InitTun(c_sTunName1);
    const int fdTun2 = tun::InitTun(c_sTunName2);

    std::cout << "Successfully connected to interfaces " << c_sTunName1 << " & " << c_sTunName2 << std::endl;

    DestroyTunnelRoutes(c_sEthName, c_sTunName1, c_sTunName2);
    CreateTunnelRoutes(c_sEthName, c_sTunName1, c_sTunName2);

    std::cout << "\033[93m" << "--------------------------------------------------------" << "\033[0m" << std::endl;
    std::cout << "\033[93m" << "Tunnel created:" << "\033[0m" << std::endl;
    std::cout << "\033[93mAPP <--> [" << c_sTunName1 << "] <==TunVis==> [" << c_sTunName2 << "] <--> [" << c_sEthName << "] <--> INTERNET\033[0m" << std::endl;
    std::cout << "\033[93m" << "--------------------------------------------------------" << "\033[0m" << std::endl;

    const std::vector<CFilterRule> arRules = filter_rules::readRules("dat/rules1.txt");
    filter_rules::displayRules(arRules);

    class CRuleTrack {
    public:
        CRuleTrack()  {}
        ~CRuleTrack() {}
    public:
        EFilterRule eRule {EFilterRule::Undefined};
        uint64_t    uValue {0U};
        bool        bTerminate {false};
        // CFilterRule *pRule {nullptr};

        // std::string sTitle;
        // uint32_t    uNr {0};
        // uint32_t    uAddress {0};
        // uint32_t    uMaskBits {0};
        // int64_t     nRuleValue {0};
        // std::string sRule;
        // std::string sNote;
    };

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
            const uint16_t uRead = tun::Read(fdTun1, buffer, sizeof(buffer));

            bool bTerminate = false;
            const CInfo info = ipv4::parseIpv4(buffer);
            const int32_t nRuleIndex = filter_rules::findLastRule(arRules, info.uDst, info.eProtocol);

            if (nRuleIndex != -1) {
                const CFilterRule &rule = arRules[nRuleIndex];
                CRuleTrack &track = arTrack[nRuleIndex];
                bTerminate = track.bTerminate;

                if (!bTerminate) {
                    print_current_time();
                    PrintOutgoingPacket(nPacketCounter, uRead, info);
                    std::cout << "\033[96m => #" << rule.uNr <<  ": " << rule.sTitle << "\033[0m";
                    std::cout << std::endl;
                }
            }

            if (!bTerminate) {
                tun::Write(fdTun2, buffer, uRead);
            }
        }

        if (FD_ISSET(fdTun2, &fdSet)) {
            ++nPacketCounter;
            const uint16_t uRead = tun::Read(fdTun2, buffer, sizeof(buffer));

            const CInfo info = ipv4::parseIpv4(buffer);
            std::time_t now = std::time(nullptr);
            bool bTerminate = false;

            const int32_t nRuleIndex = filter_rules::findLastRule(arRules, info.uSrc, info.eProtocol);
            if (nRuleIndex != -1) {
                print_current_time();

                const CFilterRule &rule = arRules[nRuleIndex];
                CRuleTrack &track = arTrack[nRuleIndex];

                switch (rule.eRuleType) {
                case EFilterRule::LimitTime: {
                    if (track.uValue == 0U) {
                        track.uValue = now;
                    } else if (now - track.uValue > rule.uRuleValue) {
                        track.bTerminate = true;
                        bTerminate = true;
                    }
                }
                    break;
                case EFilterRule::LimitDownload:
                    if (track.uValue + uRead <= rule.uRuleValue) {
                        track.uValue += uRead;
                    } else {
                        track.bTerminate = true;
                        bTerminate = true;
                    }
                    break;
                case EFilterRule::Undefined:
                default:
                    // std::cout << "ERROR: Internal error, unknown rule type" << std::endl;
                    break;
                }

                PrintIncommingPacket(nPacketCounter, uRead, info, bTerminate);

                if (nRuleIndex != -1) {
                    const CFilterRule &rule = arRules[nRuleIndex];
                    std::cout << "\033[36m";
                    std::cout << " => #" << rule.uNr <<  ": " << rule.sTitle;
                    std::cout << "\033[0m";
                }

                if (nRuleIndex != -1) {
                    const CFilterRule &rule = arRules[nRuleIndex];
                    CRuleTrack &track = arTrack[nRuleIndex];

                    switch (rule.eRuleType) {
                    case EFilterRule::LimitTime:
                        std::cout << "\033[93m => [" << (now - track.uValue) << " s]\033[0m";
                        std::cout << (bTerminate ? "\033[91m TERM\033[0m" : "\033[92m OK\033[0m");
                        break;
                    case EFilterRule::LimitDownload:
                        std::cout << "\033[95m => [" << track.uValue << " B]\033[0m";
                        std::cout << (bTerminate ? "\033[91m TERM\033[0m" : "\033[92m OK\033[0m");
                        break;
                    case EFilterRule::Undefined:
                    default:
                        // std::cout << "ERROR: Internal error, unknown rule type" << std::endl;
                        break;
                    }
                    std::cout << std::endl;
                }
            }

            if (!bTerminate) {
                tun::Write(fdTun1, buffer, uRead);
            }
        }
    }

    return 0;
}
