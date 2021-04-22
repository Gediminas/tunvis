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

void signal_callback_handler(int signum) {
   std::cout << "Program terminating " << signum << std::endl;
   DestroyTunnelRoutes(c_sEthName, c_sTunName1, c_sTunName2);
   sleep(3);
   exit(signum);
}

int main() {
    std::cout << "\033[1;33m" << "=======================================================" << "\033[0m" << std::endl;
    std::cout << "\033[1;33m" << "|  Tunnel Vission" << "\033[0m" << std::endl;
    std::cout << "\033[1;33m" << "=======================================================" << "\033[0m" << std::endl;

    signal(SIGINT, signal_callback_handler);

    const std::vector<CFilterRule> arRules = filter_rules::readRules("dat/rules1.txt");
    filter_rules::displayRules(arRules);

    const int fdTun1 = tun::InitTun(c_sTunName1);
    const int fdTun2 = tun::InitTun(c_sTunName2);

    std::cout << "Successfully connected to interfaces " << c_sTunName1 << " & " << c_sTunName2 << std::endl;

    DestroyTunnelRoutes(c_sEthName, c_sTunName1, c_sTunName2);
    CreateTunnelRoutes(c_sEthName, c_sTunName1, c_sTunName2);

    std::cout << "\033[93m" << "--------------------------------------------------------" << "\033[0m" << std::endl;
    std::cout << "\033[93m" << "Tunnel created:" << "\033[0m" << std::endl;
    std::cout << "\033[93mAPP <--> [" << c_sTunName1 << "] <==TunVis==> [" << c_sTunName2 << "] <--> [" << c_sEthName << "] <--> INTERNET\033[0m" << std::endl;
    std::cout << "\033[93m" << "--------------------------------------------------------" << "\033[0m" << std::endl;

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

            CInfo info = ipv4::parseIpv4(buffer);
            info.uSize = uRead;

            if (const CFilterRule* pRule = filter_rules::findLastRule(arRules, info.uDst)) {
                std::cout << "\033[92m";
                std::cout << nPacketCounter <<  ": " << uRead << " B";
                std::cout << " --> " << ipv4::numberToAddress(info.uDst);
                // std::cout << "  (" << ipv4::numberToAddress(info.uSrc) << ")";
                std::cout << "\033[0m";

                std::cout << "\033[96m";
                std::cout << " => #" << pRule->uNr <<  ": " << pRule->sTitle;
                std::cout << "\033[0m";
                std::cout << std::endl;
            }

            tun::Write(fdTun2, buffer, uRead);
        }

        if (FD_ISSET(fdTun2, &fdSet)) {
            ++nPacketCounter;
            const uint16_t uRead = tun::Read(fdTun2, buffer, sizeof(buffer));

            CInfo info = ipv4::parseIpv4(buffer);
            info.uSize = uRead;

            if (const CFilterRule* pRule = filter_rules::findLastRule(arRules, info.uSrc)) {
                std::cout << "\033[32m";
                std::cout << nPacketCounter <<  ": " << uRead << " B";
                std::cout << " <-- " << ipv4::numberToAddress(info.uSrc);
                // std::cout << "  (" << ipv4::numberToAddress(info.uDst) << ")";
                std::cout << "\033[0m";

                std::cout << "\033[36m";
                std::cout << " => #" << pRule->uNr <<  ": " << pRule->sTitle;
                std::cout << "\033[0m";
                std::cout << std::endl;
            }

            tun::Write(fdTun1, buffer, uRead);
        }
    }

    return 0;
}
