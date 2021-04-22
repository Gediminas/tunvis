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

constexpr int BUFSIZE {2000}; //for reading from tun/tap interface, must be >= 1500
constexpr const char *if_name1 = "tunvis1";
constexpr const char *if_name2 = "tunvis2";
constexpr int flags = IFF_TUN | IFF_NO_PI; //IFF_TAP IFF_MULTI_QUEUE
int nr = 0;

int main() {

    std::cout << "\033[1;33m" << "Tunnel-Vission started!" << "\033[0m" << std::endl;

    const std::vector<CFilterRule> arRules = readRules("dat/rules1.txt");
    std::cout << "\033[93m"  << "Rules loaded" << "\033[0m" << std::endl;
    for (const CFilterRule &rule : arRules) {
        std::cout << "\033[93m"  << "#" << rule.uNr << ":   " << rule.sTitle << "\033[0m" << std::endl;
        std::cout << "Addr: " << rule.uAddress  << std::endl;
        std::cout << "Mask: " << rule.uMaskBits << std::endl;
        std::cout << "Rule: " << rule.sRule     << std::endl;
        if (!rule.sNote.empty()) {
            std::cout << "\033[37m" << "Note: " << rule.sNote  << "\033[0m"<< std::endl;
        }
        std::cout << std::endl;
    }

    sleep(1);

    const int fdTun1  = InitializeTUN(if_name1, flags);
    const int fdTun2 = InitializeTUN(if_name2, flags);

    if (fdTun1 < 0) {
        std::cerr << "Error connecting to tun/tap interface " << if_name1 << std::endl;
        exit(1);
    }
    if (fdTun2 < 0) {
        std::cerr << "Error connecting to tun/tap interface " << if_name2 << std::endl;
        exit(1);
    }

    std::cout << "Successfully connected to interfaces " << if_name1 << " & " << if_name2 << std::endl;
    std::cout << "Creating tunnel " << if_name1 << "-" << if_name2 << std::endl;

    routing();

    char buffer[BUFSIZE];

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

            if (const CFilterRule* pRule = findLastRule(arRules, info.uDst)) {
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

            if (const CFilterRule* pRule = findLastRule(arRules, info.uSrc)) {
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
