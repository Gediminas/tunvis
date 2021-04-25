#include "Routing.h"
#include "Process.h"
#include "str_util.h"

std::string routing::GetDefaultEthName() {
    const std::string output = exec("ip route | grep default");
    std::cout << "ETH: " << output << std::endl;
    return "enp0s3";
}

void routing::CreateTunnelRoutes(const char *sEthName, const char *sTunName1, const char *sTunName2)
{
    std::cout << "Creating tunnel" << std::endl;

    const char *sEthIP = "192.168.101.137";

    system("echo 1 > /proc/sys/net/ipv4/ip_forward");
    system(str_format("echo 0 > /proc/sys/net/ipv4/conf/%s/rp_filter", sTunName1).c_str());

    // system("echo 1 > /proc/sys/net/ipv4/tcp_fwmark_accept");
    // system("echo 0 > /proc/sys/net/ipv4/conf/default/rp_filter");
    // system(str_format("echo 0 > /proc/sys/net/ipv4/conf/%s/rp_filter", sIFName1).c_str());
    // system("echo 0 > /proc/sys/net/ipv4/conf/tunvis2/rp_filter");
    // system("echo 0 r /proc/sys/net/ipv4/conf/enp0s3/rp_filter");

    system(str_format("ip link set %s up", sTunName1).c_str());
    system(str_format("ip link set %s up", sTunName2).c_str());

    system(str_format("ip addr add 10.0.1.1/24 dev %s", sTunName1).c_str());
    system(str_format("ip addr add 10.0.2.2/24 dev %s", sTunName2).c_str());

    // OUT

    // APP -> OUTPUT -> POST ---------------> normal packet rooute ------------------------ [enp0s3] --> INTERNET
    //          ^              \                                                        /   (192.168.101.137)
    //      (mark-1)            -> [tunvis1] ==copy==> [tunvis2] -> PRE -> FWD -> POST -
    //      ( =>fwmark-1)               ^                  ^         ^
    //      ( =>snat-1)           (10.0.1.1/24)     (10.0.2.2/24)   (mark-2)
    //                                                              ( =>snat-2)

    system("ip rule add fwmark 1 table 1");
    system("ip route add table 1 default via 10.0.1.1");

    system("iptables -t mangle -A OUTPUT -j MARK --set-mark 1"); //mark-1
    system(str_format("iptables -t mangle -A PREROUTING -i %s -j MARK --set-mark 2", sTunName2).c_str()); //mark-2
    system("iptables -t nat -A POSTROUTING -m mark --mark 1 -j SNAT --to-source 10.0.2.22"); //snat-1
    system(str_format("iptables -t nat -A POSTROUTING -m mark --mark 2 -j SNAT --to-source %s", sEthIP).c_str()); //snat-2

    // IN

    //                        <-- [tunvis1] <==copy== [tunvis2] <--
    //                      /         ^                   ^        \                                  .
    //                     /  (10.0.1.1/24)         (10.0.2.2/24)  POST
    //                    /                                          \                                .
    //                  PRE                                          FWD
    //                  /                                              \                              .
    // APP <- INPUT <--------------- normal packet rooute <--------------- PRE <-- [enp0s3] <-- INTERNET
    //                                                                  (dnat-1)   (192.168.101.137)

    system(str_format("iptables -t nat -A PREROUTING -i %s  -j DNAT --to-destination 10.0.2.22", sEthName).c_str()); //dnat-1



    system(str_format("iptables -I INPUT -i %s -j DROP", sEthName).c_str()); //debug

}

void routing::DestroyTunnelRoutes(const char *sEthName, const char *sTunName1, const char *sTunName2)
{
    std::cout << "Destroying tunnel (if exists)" << std::endl;

    const char *sEthIP = "192.168.101.137";

    system(str_format("iptables -D INPUT -i %s -j DROP 2>/dev/null", sEthName).c_str()); //debug

    system(str_format("iptables -t nat -D PREROUTING -i %s  -j DNAT --to-destination 10.0.2.22 2>/dev/null", sEthName).c_str());

    system(str_format("iptables -t nat -D POSTROUTING -m mark --mark 2 -j SNAT --to-source %s 2>/dev/null", sEthIP).c_str());
    system("iptables -t nat -D POSTROUTING -m mark --mark 1 -j SNAT --to-source 10.0.2.22 2>/dev/null");
    system(str_format("iptables -t mangle -D PREROUTING -i %s -j MARK --set-mark 2 2>/dev/null", sTunName2).c_str());
    system("iptables -t mangle -D OUTPUT -j MARK --set-mark 1 2>/dev/null");

    system(str_format("ip link set %s down 2>/dev/null", sTunName1).c_str());
    system(str_format("ip link set %s down 2>/dev/null", sTunName2).c_str());

    system("ip route del table 1 default via 10.0.1.1 2>/dev/null");
    system("ip rule del fwmark 1 table 1 2>/dev/null");
}

