#include "routing.h"

// #include <stdio.h>
// #include <unistd.h>
// #include <sys/types.h>
// #include <fcntl.h>
// #include <sys/time.h>
// #include <linux/if_tun.h>
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

#include <stdlib.h>

void routing()
{
    system("echo 1 > /proc/sys/net/ipv4/ip_forward");
    // system("echo 1 > /proc/sys/net/ipv4/tcp_fwmark_accept");

    // system("echo 0 > /proc/sys/net/ipv4/conf/default/rp_filter");
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
}
