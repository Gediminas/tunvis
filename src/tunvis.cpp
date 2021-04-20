#include <iostream>
#include <sstream>

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

#include "tun.h"

constexpr int BUFSIZE {2000}; //for reading from tun/tap interface, must be >= 1500
constexpr const char *if_name1 = "tunvis1";
constexpr const char *if_name2 = "tunvis2";
constexpr int flags = IFF_TUN | IFF_NO_PI; //IFF_TAP IFF_MULTI_QUEUE
int nr = 0;

class CInfo final {
public:
  CInfo()  {}
  ~CInfo() {}
public:
  uint32_t    uSrc {0};
  uint32_t    uDst {0};
  std::string sSrc;
  std::string sDst;
  uint16_t    uSize {0};
};

std::string toIpv4Address(const uint32_t uAddress) {
  const uint8_t a1 = (uint8_t) (0xFF &  uAddress);
  const uint8_t a2 = (uint8_t) (0xFF & (uAddress >> 8));
  const uint8_t a3 = (uint8_t) (0xFF & (uAddress >> 16));
  const uint8_t a4 = (uint8_t) (0xFF & (uAddress >> 24));
  std::stringstream ss;
  ss << +a1 << "." << +a2 << "." << +a3 << "." << +a4;
  return ss.str();
}

CInfo parseIpv4(const char *data) {
  const uint32_t *pFirst = (uint32_t*) data;
  CInfo info;
  info.uSrc = *(pFirst + 3);
  info.uDst = *(pFirst + 4);
  info.sSrc = toIpv4Address(info.uSrc);
  info.sDst = toIpv4Address(info.uDst);
  return info;
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
  system("echo 1 > /proc/sys/net/ipv4/tcp_fwmark_accept");

  system("echo 0 > /proc/sys/net/ipv4/conf/default/rp_filter");
  system("echo 0 > /proc/sys/net/ipv4/conf/tunvis1/rp_filter");
  // system("echo 0 > /proc/sys/net/ipv4/conf/tunvis2/rp_filter");
  // system("echo 0 > /proc/sys/net/ipv4/conf/enp0s3/rp_filter");

  system("ip link set tunvis1 up");
  system("ip link set tunvis2 up");

  system("ip addr add 10.0.1.1/24 dev tunvis1");
  system("ip addr add 10.0.2.2/24 dev tunvis2");

  // OUT

  // APP -> OUTPUT -> POST ---------------> normal packet rooute ------------------------ [enp0s3] --> INTERNET
  //          ^              \                                                        /   (192.168.101.137)
  //      (mark-1)            -> [tunvis1] ==copy==> [tunvis2] -> PRE -> FWD -> POST -
  //      ( =>fwmark-1)               ^                  ^         ^
  //      ( =>snat-1)           (10.0.1.1/24)     (10.0.2.2/24)   (mark-2)
  //                                                              ( =>snat-2)

  system("ip rule del fwmark 1 table 1");
  system("ip rule add fwmark 1 table 1");

  system("ip route del table 1 default via 10.0.1.1");
  system("ip route add table 1 default via 10.0.1.1");

  system("iptables -t mangle -D OUTPUT -j MARK --set-mark 1");
  system("iptables -t mangle -A OUTPUT -j MARK --set-mark 1"); //mark-1

  system("iptables -t mangle -D PREROUTING -i tunvis2 -j MARK --set-mark 2");
  system("iptables -t mangle -A PREROUTING -i tunvis2 -j MARK --set-mark 2"); //mark-2

  system("iptables -t nat -D POSTROUTING -m mark --mark 1 -j SNAT --to-source 10.0.2.22");
  system("iptables -t nat -A POSTROUTING -m mark --mark 1 -j SNAT --to-source 10.0.2.22"); //snat-1

  system("iptables -t nat -D POSTROUTING -m mark --mark 2 -j SNAT --to-source 192.168.101.137");
  system("iptables -t nat -A POSTROUTING -m mark --mark 2 -j SNAT --to-source 192.168.101.137"); //snat-2

  // IN

  //                        <-- [tunvis1] <==copy== [tunvis2] <--
  //                      /         ^                   ^        \                                  .
  //                     /  (10.0.1.1/24)         (10.0.2.2/24)  POST
  //                    /                                          \                                .
  //                  PRE                                          FWD
  //                  /                                              \                              .
  // APP <- INPUT <--------------- normal packet rooute <--------------- PRE <-- [enp0s3] <-- INTERNET
  //                                                                  (dnat-1)   (192.168.101.137)

  system("iptables -t nat -D PREROUTING -i enp0s3  -j DNAT --to-destination 10.0.2.22");
  system("iptables -t nat -A PREROUTING -i enp0s3  -j DNAT --to-destination 10.0.2.22"); //dnat-1




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
      const uint16_t uRead = cread(tun_in_fd, buffer, sizeof(buffer));

      CInfo info = parseIpv4(buffer);
      info.uSize = uRead;

      std::cout << "I-" << ++nr <<  ": " << uRead << " B";
      std::cout << "  " << info.sSrc << " --> " << info.sDst;
      std::cout << std::endl;

      cwrite(tun_out_fd, buffer, uRead);
    }

    if (FD_ISSET(tun_out_fd, &rd_set)) {
      const uint16_t uRead = cread(tun_out_fd, buffer, sizeof(buffer));

      CInfo info = parseIpv4(buffer);
      info.uSize = uRead;

      std::cout << "O-" << ++nr <<  ": " << uRead << " B";
      std::cout << "  " << info.sSrc << " --> " << info.sDst;
      std::cout << std::endl;

      cwrite(tun_in_fd, buffer, uRead);
    }
  }

  return 0;
}
// https://erg.abdn.ac.uk/users/gorry/course/inet-pages/packet-dec2.html
// https://www.techrepublic.com/article/exploring-the-anatomy-of-a-data-packet/
//
