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
constexpr int flags = IFF_TUN | IFF_NO_PI; //IFF_TAP IFF_MULTI_QUEUE
int nr = 0;

void print_ip(unsigned int ip)
{
  unsigned char bytes[4];
  bytes[0] = ip & 0xFF;
  bytes[1] = (ip >> 8) & 0xFF;
  bytes[2] = (ip >> 16) & 0xFF;
  bytes[3] = (ip >> 24) & 0xFF;
  // printf("%d.%d.%d.%d\n", bytes[3], bytes[2], bytes[1], bytes[0]);
  std::cout << (int)bytes[3] << "." << (int)bytes[2] << "." << (int)bytes[1] << "." << (int)bytes[0];
}

int main() {

  std::cout << "Tunnel-Vission started!" << std::endl;

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

  system("echo 1 > /proc/sys/net/ipv4/ip_forward");
  system("ip link set tun11 up");
  system("ip link set tun12 up");
  system("ip addr add 10.77.11.11/24 dev tun11");
  system("ip addr add 10.77.12.12/24 dev tun12");
  system("ip route add default via 10.77.11.11");

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

    if (FD_ISSET(tun_in_fd, &rd_set)) {
      const uint16_t nread = cread(tun_in_fd, buffer, sizeof(buffer));
      std::cout << "I-" << ++nr <<  ": " << nread << " B";

      const int x = *((int*)(buffer));
      std::cout << " / "; print_ip(x); std::cout << std::endl;

      cwrite(tun_out_fd, buffer, nread);
    }

    if (FD_ISSET(tun_out_fd, &rd_set)) {
      const uint16_t nread = cread(tun_out_fd, buffer, sizeof(buffer));
      std::cout << "O-" << ++nr <<  ": " << nread << " B";

      const int x = *((int*)(buffer));
      std::cout << " / "; print_ip(x); std::cout << std::endl;

      cwrite(tun_in_fd, buffer, nread);
    }
  }

  return 0;
}
