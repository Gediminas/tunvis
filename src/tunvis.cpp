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

constexpr int32_t     c_nBufferSize {2000}; //for reading from tun/tap interface, must be >= 1500
constexpr const char *c_sIFName1 = "tunvis1";
constexpr const char *c_sIFName2 = "tunvis2";
constexpr int flags = IFF_TUN | IFF_NO_PI; //IFF_TAP IFF_MULTI_QUEUE
int nr = 0;

int main() {

    std::cout << "\033[1;33m" << "Tunnel-Vission started!" << "\033[0m" << std::endl;

    const std::vector<CFilterRule> arRules = filter_rules::readRules("dat/rules1.txt");
    filter_rules::displayRules(arRules);

    const int fdTun1  = InitializeTUN(c_sIFName1, flags);
    if (fdTun1 < 0) {
        std::cerr << "Error connecting to tun/tap interface " << c_sIFName1 << std::endl;
        exit(1);
    }

    const int fdTun2 = InitializeTUN(c_sIFName2, flags);
    if (fdTun2 < 0) {
        std::cerr << "Error connecting to tun/tap interface " << c_sIFName2 << std::endl;
        exit(1);
    }

    std::cout << "Successfully connected to interfaces " << c_sIFName1 << " & " << c_sIFName2 << std::endl;
    std::cout << "Creating tunnel " << c_sIFName1 << "-" << c_sIFName2 << std::endl;

    routing();

    char buffer[c_nBufferSize];

    /* use select() to handle two descriptors at once */
    const int maxfd = (fdTun1 > fdTun2) ? fdTun1 : fdTun2;

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
            const uint16_t uRead = cread(fdTun1, buffer, sizeof(buffer));

            CInfo info = parseIpv4(buffer);
            info.uSize = uRead;

            if (const CFilterRule* pRule = filter_rules::findLastRule(arRules, info.uDst)) {
                std::cout << "\033[92m";
                std::cout << ++nr <<  ": " << uRead << " B";
                std::cout << " --> " << numberToAddress(info.uDst);
                // std::cout << "  (" << numberToAddress(info.uSrc) << ")";
                std::cout << "\033[0m";

                std::cout << "\033[96m";
                std::cout << " => #" << pRule->uNr <<  ": " << pRule->sTitle;
                std::cout << "\033[0m";
                std::cout << std::endl;
            }

            cwrite(fdTun2, buffer, uRead);
        }

        if (FD_ISSET(fdTun2, &fdSet)) {
            const uint16_t uRead = cread(fdTun2, buffer, sizeof(buffer));

            CInfo info = parseIpv4(buffer);
            info.uSize = uRead;

            if (const CFilterRule* pRule = filter_rules::findLastRule(arRules, info.uSrc)) {
                std::cout << "\033[32m";
                std::cout << ++nr <<  ": " << uRead << " B";
                std::cout << " <-- " << numberToAddress(info.uSrc);
                // std::cout << "  (" << numberToAddress(info.uDst) << ")";
                std::cout << "\033[0m";

                std::cout << "\033[36m";
                std::cout << " => #" << pRule->uNr <<  ": " << pRule->sTitle;
                std::cout << "\033[0m";
                std::cout << std::endl;
            }

            cwrite(fdTun1, buffer, uRead);
        }
    }

    return 0;
}
// https://erg.abdn.ac.uk/users/gorry/course/inet-pages/packet-dec2.html
// https://www.techrepublic.com/article/exploring-the-anatomy-of-a-data-packet/
//
