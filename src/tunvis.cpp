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

#include "tun.h"

constexpr int BUFSIZE {2000}; //for reading from tun/tap interface, must be >= 1500
constexpr const char *if_name1 = "tunvis1";
constexpr const char *if_name2 = "tunvis2";
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
  system("echo 1 > /proc/sys/net/ipv4/tcp_fwmark_accept");
  system("echo 1 > /proc/sys/net/ipv4/tcp_syncookies");
  system("echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts");
  system("echo 0 > /proc/sys/net/ipv4/conf/all/accept_redirects");
  system("echo 0 > /proc/sys/net/ipv4/conf/all/accept_source_route");
  system("echo 1 > /proc/sys/net/ipv4/conf/all/log_martians");
  system("echo 1 > /proc/sys/net/ipv4/icmp_ignore_bogus_error_responses");
  system("echo 0 > /proc/sys/net/ipv4/conf/all/send_redirects");

  system("echo 0 > /proc/sys/net/ipv4/conf/default/rp_filter");
  system("echo 0 > /proc/sys/net/ipv4/conf/tunvis1/rp_filter");
  system("echo 0 > /proc/sys/net/ipv4/conf/tunvis2/rp_filter");
  system("echo 0 > /proc/sys/net/ipv4/conf/enp0s3/rp_filter");

  system("echo 1 > /proc/sys/net/ipv4/conf/default/accept_local");
  system("echo 1 > /proc/sys/net/ipv4/conf/tunvis1/accept_local");
  system("echo 1 > /proc/sys/net/ipv4/conf/tunvis2/accept_local");
  system("echo 1 > /proc/sys/net/ipv4/conf/enp0s3/accept_local");

  system("echo 1 > /proc/sys/net/ipv4/conf/default/accept_redirects");
  system("echo 1 > /proc/sys/net/ipv4/conf/tunvis1/accept_redirects");
  system("echo 1 > /proc/sys/net/ipv4/conf/tunvis2/accept_redirects");
  system("echo 1 > /proc/sys/net/ipv4/conf/enp0s3/accept_redirects");

  system("echo 0 > /proc/sys/net/ipv4/conf/default/accept_source_route");
  system("echo 0 > /proc/sys/net/ipv4/conf/tunvis1/accept_source_route");
  system("echo 0 > /proc/sys/net/ipv4/conf/tunvis2/accept_source_route");
  system("echo 0 > /proc/sys/net/ipv4/conf/enp0s3/accept_source_route");

  system("echo 1 > /proc/sys/net/ipv4/conf/default/forwarding");
  system("echo 1 > /proc/sys/net/ipv4/conf/tunvis1/forwarding");
  system("echo 1 > /proc/sys/net/ipv4/conf/tunvis2/forwarding");
  system("echo 1 > /proc/sys/net/ipv4/conf/enp0s3/forwarding");

  system("echo 1 > /proc/sys/net/ipv4/conf/default/mc_forwarding");
  system("echo 1 > /proc/sys/net/ipv4/conf/tunvis1/mc_forwarding");
  system("echo 1 > /proc/sys/net/ipv4/conf/tunvis2/mc_forwarding");
  system("echo 1 > /proc/sys/net/ipv4/conf/enp0s3/mc_forwarding");

  system("echo 1 > /proc/sys/net/ipv4/conf/default/bc_forwarding");
  system("echo 1 > /proc/sys/net/ipv4/conf/tunvis1/bc_forwarding");
  system("echo 1 > /proc/sys/net/ipv4/conf/tunvis2/bc_forwarding");
  system("echo 1 > /proc/sys/net/ipv4/conf/enp0s3/bc_forwarding");


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
  system("iptables -t mangle -A OUTPUT -j MARK --set-mark 1"); // Mark 1

  system("iptables -t mangle -D PREROUTING -i tunvis2 -j MARK --set-mark 2");
  system("iptables -t mangle -A PREROUTING -i tunvis2 -j MARK --set-mark 2"); // Mark 2

  system("iptables -t nat -D POSTROUTING -m mark --mark 1 -j SNAT --to-source 10.0.2.22");
  system("iptables -t nat -A POSTROUTING -m mark --mark 1 -j SNAT --to-source 10.0.2.22"); //snat1

  system("iptables -t nat -D POSTROUTING -m mark --mark 2 -j SNAT --to-source 192.168.101.137");
  system("iptables -t nat -A POSTROUTING -m mark --mark 2 -j SNAT --to-source 192.168.101.137"); //snat2

  // IN

  //                        <-- [tunvis1] <==copy== [tunvis2] <--
  //                      /         ^                   ^        \                                  .
  //                     /  (10.0.1.1/24)         (10.0.2.2/24)  POST
  //          (dnat-2)  /                                          \                                .
  //                  PRE                                          FWD
  //                  /                                              \                              .
  // APP <- INPUT <--------------- normal packet rooute <--------------- PRE <-- [enp0s3] <-- INTERNET
  //                                                                  (dnat-1)   (192.168.101.137)

  system("iptables -t nat -D PREROUTING -i enp0s3  -j DNAT --to-destination 10.0.2.22");
  system("iptables -t nat -A PREROUTING -i enp0s3  -j DNAT --to-destination 10.0.2.22"); //dnat1

  // system("iptables -t nat -D PREROUTING -i tunvis1 -j DNAT --to-destination 192.168.101.137");
  // system("iptables -t nat -A PREROUTING -i tunvis1 -j DNAT --to-destination 192.168.101.137"); //dnat2



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
