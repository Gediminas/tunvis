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

  system("ip addr add 10.1.1.1/24 dev tunvis1");
  system("ip addr add 10.2.2.2/24 dev tunvis2");

  //OUT
  // APP -> OUTPUT -> POST ---------------> normal packet rooute ------------------------ [enp0s3] --> INTERNET
  //          ^              \                                                        /
  //       add-mark-1         -> [tunvis1] ==copy==> [tunvis2] -> PRE -> FWD -> POST -
  //                                 ^                   ^         ^
  //                         (10.77.11.11)       (10.77.12.12)   del-mark-1
  //
  //

  system("ip rule del fwmark 1 table 1 prio 1");
  system("ip rule add fwmark 1 table 1 prio 1");

  system("ip route del table 1 default via 10.1.1.1");
  system("ip route add table 1 default via 10.1.1.1");

  system("iptables -t mangle -D OUTPUT -j MARK --set-mark 1");   // Add mark 1
  system("iptables -t mangle -A OUTPUT -j MARK --set-mark 1");   // Add mark 1

  // system("iptables -t mangle -D PREROUTING -i tunvis2 -j MARK --set-mark 0/1"); // Remove mark 1
  // system("iptables -t mangle -I PREROUTING -i tunvis2 -j MARK --set-mark 0/1"); // Remove mark 1

  //Just for visual
  system("iptables -t mangle -D PREROUTING -i tunvis2 -j MARK --set-mark 9"); // Remove mark 1
  system("iptables -t mangle -A PREROUTING -i tunvis2 -j MARK --set-mark 9"); // Remove mark 1


  //IN
  system("ip rule del fwmark 2 table 2 prio 2");
  system("ip rule add fwmark 2 table 2 prio 2");

  system("ip route del table 2 default via 10.2.2.2");
  system("ip route add table 2 default via 10.2.2.2");

  system("iptables -t mangle -D PREROUTING -i enp0s3 -j MARK --set-mark 2");
  system("iptables -t mangle -A PREROUTING -i enp0s3 -j MARK --set-mark 2");

  //system("iptables -t nat -I PREROUTING 1 -d 192.168.101.137 -j DNAT --to-destination 10.2.2.222");


  //OK
  // echo "200 TUNVIS" >> /etc/iproute2/rt_tables
  // ip route add default table TUNVIS via 192.168.101.1
  // ip rule add fwmark 42 table TUNVIS
  // iptables -t mangle -I PREROUTING -d 0.0.0.0/0 -j MARK --set-mark 42


  //xz
  // iptables -F
  // iptables -F -t nat
  // iptables -t nat -I POSTROUTING -d 0.0.0.0/0 -j SNAT --to-source 10.77.12.12
  // iptables -t mangle -I PREROUTING -i tunvis2 -d 0.0.0.0/0 -j MARK --set-mark 42

  // iptables -t nat -A POSTROUTING -m mark --mark 42 -j SNAT --to-source 10.77.12.12


// You can use iptables to redirect your eth1 traffic to tun0:
// sudo iptables -t nat -A POSTROUTING -o tun0 -j MASQUERADE
// sudo iptables -A FORWARD -i tun0 -o eth1 -m state --state RELATED,ESTABLISHED -j ACCEPT
// sudo iptables -A FORWARD -i eth1 -o tun0 -j ACCEPT

  // https://wiki.archlinux.org/index.php/Network_bridge
  // system("ip link add name br_tunvis type bridge");
  // system("ip link set br_tunvis up");
  // system("ip link set enp0s3 up");
  // system("ip link set enp0s3 master br_tunvis");
  // system("ip link set tunvis2 master br_tunvis");
  // system("bridge link");



  // Route all traffic through TUN interface
  // https://superuser.com/questions/1614666/route-all-traffic-through-tun-interface




  // ip link add veth0 type veth peer name veth1 netns blue

  // system("ip rule del from 192.168.101.137 lookup 2");
  // system("ip rule del from 10.77.11.11 lookup 2");

  // system("iptables -t nat -I POSTROUTING 1 -s 10.77.11.11 -j SNAT --to-source 192.168.101.1");
  // system("iptables -t nat -I PREROUTING 1 -d 192.168.101.1 -j DNAT --to-destination 10.77.11.11");

  // system("iptables -t nat -I POSTROUTING 1 -s 192.168.101.137 -j SNAT --to-source 10.77.11.11");
  // system("iptables -t nat -I PREROUTING 1 -d 10.77.11.11 -j DNAT --to-destination 192.168.101.137");




  // https://serverfault.com/questions/356165/forwarding-traffic-from-tun-device-c-backend-to-the-default-gateway?newreg=90f16a8eec8a4dc28c94af5aef531881
  // echo 1 > /proc/sys/net/ipv4/conf/tun0/accept_local


  // system("iptables -F");
  // system("iptables -F -t nat");
  // system("iptables -F -t mangle");

  // system("iptables -t nat -A POSTROUTING -o enp0s3 -j MASQUERADE");
  // system("iptables -A FORWARD -i tunvis1 -j ACCEPT");

  // https://blog.scottlowe.org/2013/09/04/introducing-linux-network-namespaces/
  // system("ip netns add tunvis");
  // system("ip link set tunvis2 netns tunvis");
  // system("ip netns exec tunvis ip link list");


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
