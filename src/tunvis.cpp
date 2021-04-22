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

    const int tun_in_fd  = InitializeTUN(if_name1, flags);
    const int tun_out_fd = InitializeTUN(if_name2, flags);

    if (tun_in_fd < 0) {
        std::cerr << "Error connecting to tun/tap interface " << if_name1 << std::endl;
        exit(1);
    }
    if (tun_out_fd < 0) {
        std::cerr << "Error connecting to tun/tap interface " << if_name2 << std::endl;
        exit(1);
    }

    std::cout << "Successfully connected to interfaces " << if_name1 << " & " << if_name2 << std::endl;
    std::cout << "Creating tunnel " << if_name1 << "-" << if_name2 << std::endl;

    routing();

    char buffer[BUFSIZE];

    /* use select() to handle two descriptors at once */
    const int maxfd = (tun_in_fd > tun_out_fd) ? tun_in_fd : tun_out_fd;

    while(1) {
        fd_set rd_set;
        FD_ZERO(&rd_set);
        FD_SET(tun_in_fd, &rd_set);
        FD_SET(tun_out_fd, &rd_set);

        const int ret = select(maxfd + 1, &rd_set, NULL, NULL, NULL);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("select()");
            exit(1);
        }

        if (FD_ISSET(tun_out_fd, &rd_set)) {
            const uint16_t uRead = cread(tun_out_fd, buffer, sizeof(buffer));

            CInfo info = parseIpv4(buffer);
            info.uSize = uRead;

            if (const CFilterRule* pRule = findLastRule(arRules, info.uSrc)) {
                std::cout << "\033[36m";
                std::cout << ++nr <<  "-O: " << uRead << " B";
                std::cout << " --> " << numberToAddress(info.uSrc);
                std::cout << "  (" << numberToAddress(info.uDst) << ") => ";
                std::cout << "\033[0m";
                // std::cout << std::endl;

                std::cout << "\033[93m";
                std::cout << "* rule " << pRule->uNr <<  ": " << pRule->sTitle;
                std::cout << "\033[0m";
                std::cout << std::endl;
            }

            cwrite(tun_in_fd, buffer, uRead);
        }

        if (FD_ISSET(tun_in_fd, &rd_set)) {
            const uint16_t uRead = cread(tun_in_fd, buffer, sizeof(buffer));

            CInfo info = parseIpv4(buffer);
            info.uSize = uRead;

            // std::cout << "\033[32m";
            // std::cout << ++nr <<  "-I: " << uRead << " B";
            // std::cout << " <-- " << numberToAddress(info.uDst);
            // std::cout << "  (" << numberToAddress(info.uSrc) << ")";
            // // std::cout << "  " << numberToAddress(info.uSrc) << " <-- . " << numberToAddress(info.uDst);
            // std::cout << "\033[0m";
            // std::cout << std::endl;

            cwrite(tun_out_fd, buffer, uRead);
        }
    }

    return 0;
}
// https://erg.abdn.ac.uk/users/gorry/course/inet-pages/packet-dec2.html
// https://www.techrepublic.com/article/exploring-the-anatomy-of-a-data-packet/
//
