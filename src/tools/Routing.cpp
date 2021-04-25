#include "Routing.h"

#include "Process.h"
#include "StrUtil.h"

std::string routing::GetDefaultEthName() {
    const std::string sCmd = "ip route get 1.2.3.4";
    const std::string sOutput = exec(sCmd.c_str());
    const std::vector<std::string> arToken = explode(sOutput, " ");
    if (arToken.size() < 5) {
        std::cerr << "\033[91m" << "ERROR: Cannot parse command '" << sCmd << "'" << " output '" << sOutput << "'" << "\033[0m" << std::endl;
        exit(-3);
    }
    return arToken[4];
}

std::string routing::GetIPByDev(const char *sDev) {
    const std::string sCmd = str_format("ip addr show %s | grep inet | grep %s", sDev, sDev);
    const std::string sOutput = exec(sCmd.c_str());
    const std::vector<std::string> arToken1 = explode(sOutput, " ");
    if (arToken1.size() < 2) {
        std::cerr << "\033[91m" << "ERROR: Cannot parse (1) command '" << sCmd << "'" << " output '" << sOutput << "'" <<"\033[0m" << std::endl;
        exit(-3);
    }
    const std::vector<std::string> arToken2 = explode(arToken1[1], "/");
    if (!arToken2.size()) {
        std::cerr << "\033[91m" << "ERROR: Cannot parse (2) command '" << sCmd << "'" << " output '" << sOutput << "'" << "\033[0m" << std::endl;
        exit(-3);
    }
    return arToken2[0];
}

void routing::CreateTunnelRoutes(const char *sTunName1, const char *sTunName2, const char *sEthName, const char *sEthIP)
{
    execv(           "echo 1 > /proc/sys/net/ipv4/ip_forward");
    execv(str_format("echo 0 > /proc/sys/net/ipv4/conf/%s/rp_filter", sTunName1).c_str());
    // execv(        "echo 1 > /proc/sys/net/ipv4/tcp_fwmark_accept");

    execv(str_format("ip link set %s up", sTunName1).c_str());
    execv(str_format("ip link set %s up", sTunName2).c_str());

    execv(str_format("ip addr add 10.0.1.1/24 dev %s", sTunName1).c_str());
    execv(str_format("ip addr add 10.0.2.2/24 dev %s", sTunName2).c_str());

    // OUT

    // APP -> OUTPUT -> POST ---------------> normal packet rooute ------------------------ [eth0] --> INTERNET
    //          ^              \                                                        /   (192.168.101.137)
    //      (mark-1)            -> [tunvis1] ==copy==> [tunvis2] -> PRE -> FWD -> POST -
    //      ( =>fwmark-1)               ^                  ^         ^
    //      ( =>snat-1)           (10.0.1.1/24)     (10.0.2.2/24)   (mark-2)
    //                                                              ( =>snat-2)

    execv(           "ip rule add fwmark 1 table 1");              //fwmark-1
    execv(           "ip route add table 1 default via 10.0.1.1"); //fwmark-1

    execv(           "iptables -t mangle -A OUTPUT -j MARK --set-mark 1"); //mark-1
    execv(str_format("iptables -t mangle -A PREROUTING -i %s -j MARK --set-mark 2", sTunName2).c_str());         //mark-2
    execv(           "iptables -t nat -A POSTROUTING -m mark --mark 1 -j SNAT --to-source 10.0.2.22");           //snat-1
    execv(str_format("iptables -t nat -A POSTROUTING -m mark --mark 2 -j SNAT --to-source %s", sEthIP).c_str()); //snat-2

    // IN

    //                        <-- [tunvis1] <==copy== [tunvis2] <--
    //                      /         ^                   ^        \                                  .
    //                     /  (10.0.1.1/24)         (10.0.2.2/24)  POST
    //                    /                                          \                                .
    //                  PRE                                          FWD
    //                  /                                drop-1        \                              .
    // APP <- INPUT <--------------- normal packet rooute <--------------- PRE <-- [eth0] <-- INTERNET
    //                                                                  (dnat-1)   (192.168.101.137)

    execv(str_format("iptables -t nat -A PREROUTING -i %s  -j DNAT --to-destination 10.0.2.22", sEthName).c_str()); //dnat-1


    // BLOCK OLD IN
    execv(str_format("iptables -I INPUT -i %s -j DROP", sEthName).c_str()); //drop-1

}

void routing::DestroyTunnelRoutes(const char *sTunName1, const char *sTunName2, const char *sEthName, const char *sEthIP)
{
    execv(str_format("iptables -D INPUT -i %s -j DROP 2>/dev/null", sEthName).c_str());

    execv(str_format("iptables -t nat -D PREROUTING -i %s  -j DNAT --to-destination 10.0.2.22 2>/dev/null", sEthName).c_str());

    execv(str_format("iptables -t nat -D POSTROUTING -m mark --mark 2 -j SNAT --to-source %s 2>/dev/null", sEthIP).c_str());
    execv(           "iptables -t nat -D POSTROUTING -m mark --mark 1 -j SNAT --to-source 10.0.2.22 2>/dev/null");
    execv(str_format("iptables -t mangle -D PREROUTING -i %s -j MARK --set-mark 2 2>/dev/null", sTunName2).c_str());
    execv(           "iptables -t mangle -D OUTPUT -j MARK --set-mark 1 2>/dev/null");

    execv(str_format("ip link set %s down 2>/dev/null", sTunName1).c_str());
    execv(str_format("ip link set %s down 2>/dev/null", sTunName2).c_str());

    execv(           "ip route del table 1 default via 10.0.1.1 2>/dev/null");
    execv(           "ip rule del fwmark 1 table 1 2>/dev/null");
}
