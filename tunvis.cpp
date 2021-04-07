#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <stdarg.h>

#include "tools/tun.h"

constexpr int BUFSIZE {2000}; //for reading from tun/tap interface, must be >= 1500
constexpr const char *if_name1 = "tun11";
constexpr const char *if_name2 = "tun12";
constexpr int flags = IFF_TUN | IFF_NO_PI; //IFF_TAP

int main() {

  std::cout << "Tunnel-Vission started!" << std::endl;

  const int tun_in_fd = InitializeTUN(if_name1, flags);
  if (tun_in_fd < 0) {
    std::cerr << "Error connecting to tun/tap interface " << if_name1 << std::endl;
    exit(1);
  }
  std::cout << "Successfully connected to interface " << if_name1 << std::endl;

  const int tun_out_fd = InitializeTUN(if_name2, flags);
  if (tun_out_fd < 0) {
    std::cerr << "Error connecting to tun/tap interface " << if_name2 << std::endl;
    exit(1);
  }
  std::cout << "Successfully connected to interface " << if_name2 << std::endl;

  /* Server, wait for connections */

  char buffer[BUFSIZE];

  /* use select() to handle two descriptors at once */
  const int maxfd = (tun_in_fd > tun_out_fd) ? tun_in_fd : tun_out_fd;

  // uint16_t nread, nwrite, plength;

  while(1) {
    fd_set rd_set;
    FD_ZERO(&rd_set);
    FD_SET(tun_in_fd, &rd_set);
    FD_SET(tun_out_fd, &rd_set);

    const int ret = select(maxfd + 1, &rd_set, NULL, NULL, NULL);
    std::cout << ret << std::endl;
    if (ret < 0) {
      if (errno == EINTR) {
        continue;
      }
      perror("select()");
      exit(1);
    }

    if( FD_ISSET(tun_in_fd, &rd_set) ) {
      std::cout << "IN" << std::endl;
      const uint16_t l = read(tun_in_fd, buffer, sizeof(buffer));
      write(tun_out_fd, buffer, l);
    }

    if( FD_ISSET(tun_out_fd, &rd_set) ) {
      std::cout << "OUT" << std::endl;
      const uint16_t l = read(tun_out_fd, buffer, sizeof(buffer));
      write(tun_in_fd, buffer, l);
    }



    // if (FD_ISSET(tun_fd, &rd_set)) {
    //   /* data from tun/tap: just read it and write it to the network */

    //   nread = cread(tun_fd, buffer, BUFSIZE);

    //   tap2net++;
    //   std::cout << "TAP2NET " << tap2net << ": Read " << nread << " bytes from the tap interface" << std::endl;

    //   /* write length + packet */
    //   plength = htons(nread);
    //   nwrite = cwrite(net_fd, (char *)&plength, sizeof(plength));
    //   nwrite = cwrite(net_fd, buffer, nread);

    //   std::cout << "TAP2NET " << tap2net << ": Written " << nwrite << " bytes to the network" << std::endl;
    // }

    // if (FD_ISSET(net_fd, &rd_set)) {
    //   /* data from the network: read it, and write it to the tun/tap interface.
    //    * We need to read the length first, and then the packet */

    //   /* Read length */
    //   nread = read_n(net_fd, (char *)&plength, sizeof(plength));
    //   if(nread == 0) {
    //     /* ctrl-c at the other end */
    //     break;
    //   }

    //   net2tap++;

    //   /* read packet */
    //   nread = read_n(net_fd, buffer, ntohs(plength));
    //   std::cout << "NET2TAP " << net2tap << ": Read " << nread << " bytes from the network" << std::endl;

    //   /* now buffer[] contains a full packet or frame, write it into the tun/tap interface */
    //   nwrite = cwrite(tun_fd, buffer, nread);
    //   std::cout << "NET2TAP " << net2tap << ": Written " << nwrite << " bytes to the tap interface" << std::endl;
    // }
  }

  return(0);
}
