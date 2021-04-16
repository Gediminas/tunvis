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

#include "tun.h"

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
  system("echo 1 > /proc/sys/net/ipv4/tcp_syncookies");
  system("echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts");
  system("echo 0 > /proc/sys/net/ipv4/conf/all/accept_redirects");
  system("echo 0 > /proc/sys/net/ipv4/conf/all/accept_source_route");
  system("echo 1 > /proc/sys/net/ipv4/conf/all/log_martians");
  system("echo 1 > /proc/sys/net/ipv4/icmp_ignore_bogus_error_responses");
  system("echo 0 > /proc/sys/net/ipv4/conf/all/send_redirects");

  system("echo 0 > /proc/sys/net/ipv4/conf/default/rp_filter");
  system("echo 0 > /proc/sys/net/ipv4/conf/tun11/rp_filter");
  system("echo 0 > /proc/sys/net/ipv4/conf/tun12/rp_filter");

  system("echo 1 > /proc/sys/net/ipv4/conf/tun11/accept_local");
  system("echo 1 > /proc/sys/net/ipv4/conf/tun12/accept_local");

  // system("iptables -F");
  // system("iptables -F -t nat");
  // system("iptables -F -t mangle");

  // system("iptables --table nat --append POSTROUTING --out-interface eth0 -j MASQUERADE");
  // system("iptables --append FORWARD --in-interface tun0 -j ACCEPT");

  // https://blog.scottlowe.org/2013/09/04/introducing-linux-network-namespaces/
  // system("ip netns add tunvis");
  // system("ip link set tun12 netns tunvis");
  // system("ip netns exec tunvis ip link list");




  system("ip link set tun11 up");
  system("ip link set tun12 up");

  system("ip addr add 10.77.11.11/24 dev tun11");
  system("ip addr add 10.77.12.12/24 dev tun12");

  // system("ip rule del fwmark 42 table TUNVIS");
  // system("ip rule add fwmark 42 table TUNVIS");

  system("ip route add default via 10.77.11.11");
  // system("ip route add table TUNVIS default via 192.168.101.1");

  // system("iptables -t mangle -D PREROUTING -i tun12 -j MARK --set-mark 42");
  // system("iptables -t mangle -I PREROUTING -i tun12 -j MARK --set-mark 42");

  // system("sudo iptables -t nat -D POSTROUTING -j SNAT --to-source 192.168.101.137");
  // system("sudo iptables -t nat -A POSTROUTING -j SNAT --to-source 192.168.101.137");


  //bridge
  // system("ip link add br0 type bridge");
  // system("ip link set tun12 master br0");
  // system("ip link set dev enp0s3 down");
  // system("ip addr flush dev enp0s3 ");
  // system("ip link set dev enp0s3 up");
  // system("ip link set enp0s3 master br0");
  // system("ip link set dev br0 up");

  //OK
  // echo "200 TUNVIS" >> /etc/iproute2/rt_tables
  // ip route add default table TUNVIS via 192.168.101.1
  // ip rule add fwmark 42 table TUNVIS
  // iptables -t mangle -I PREROUTING -d 0.0.0.0/0 -j MARK --set-mark 42


  //xz
  // iptables -F
  // iptables -F -t nat
  // iptables -t nat -I POSTROUTING -d 0.0.0.0/0 -j SNAT --to-source 10.77.12.12
  // iptables -t mangle -I PREROUTING -i tun12 -d 0.0.0.0/0 -j MARK --set-mark 42

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
  // system("ip link set tun12 master br_tunvis");
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
