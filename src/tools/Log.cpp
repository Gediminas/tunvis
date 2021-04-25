#include "Log.h"

#include "IPv4.h"
#include "Rules.h"
#include "Tracking.h"

namespace internal {
    constexpr const char *c_sByteUnits = "kmgt"; // => kb, mb, gb, tb

    static inline std::string GetFormatedBytes(uint64_t uBytes, char sUnit) {
        //sUnits - can be b, k, m, g, t (=> b, kb, mb, gb, tb)
        if (sUnit == 'b') {
            std::stringstream ss;
            ss << uBytes;
            return ss.str();
        }
        double dHuman = uBytes;
        for (const char *pUnit = c_sByteUnits; pUnit; ++pUnit) {
            dHuman /= 1024.0;
            if (*pUnit == sUnit) {
                std::stringstream ss;
                ss << std::fixed << std::setprecision(3) << dHuman;
                return ss.str();
            }
        }
        return "Invalid";
    }

    static inline std::string GetFormatedTime(uint64_t uSeconds) {
        std::stringstream ss;
        if (uSeconds % 3600 == 0) {
            ss << (uSeconds / 3600) << "h";
        } else if (uSeconds % 60 == 0) {
            ss << (uSeconds / 60) << "m";
        } else {
            ss << uSeconds << "s";
        }
        return ss.str();
    }
};

void PrintUsage(const std::string sAppName) {
    std::cerr << "Usage: " << sAppName << " <option(s)> RULES_FILE\n"
              << "Options:\n"
              << "  -h,--help                - Show this help message\n"
              << "  -i,--interface INTERFACE - Network Interface"
              << std::endl;
}

void PrintAppTitle(){
    std::cout << std::endl;
    std::cout << "\033[1;33m" << "=====================================================================" << "\033[0m" << std::endl;
    std::cout << "\033[1;33m" << "***                        Tunnel Vission                         ***" << "\033[0m" << std::endl;
    std::cout << "\033[1;33m" << "=====================================================================" << "\033[0m" << std::endl;
}

void PrintTunnel(const char *sEthName, const char *sTunName1, const char *sTunName2){
    std::cout << "\033[93m" << "---------------------------------------------------------------------" << "\033[0m" << std::endl;
    std::cout << "\033[93m" << "Tunnel created:" << "\033[0m" << std::endl;
    std::cout << "\033[93mAPP <--> [" << sTunName1 << "] <==TunVis==> [" << sTunName2 << "] <--> [" << sEthName << "] <--> INTERNET\033[0m" << std::endl;
    std::cout << "\033[93m" << "---------------------------------------------------------------------" << "\033[0m" << std::endl;
}

void PrintCurrentDateTime() {
    std::time_t t = std::time(nullptr);
    char sTime[100];
    if (std::strftime(sTime, sizeof(sTime), "%Y-%m-%d %H:%M:%S", std::localtime(&t))) {
        std::cout << "\033[90m" << sTime << "\033[0m";
    }
    std::cout << " ";
}

void PrintTraffic(int64_t nPacketCounter, int16_t uRead, const CIpv4Packet &packet, bool bAccept, bool bIncommingConnection) {
    std::cout << (bIncommingConnection ? "\033[32m" : "\033[92m");
    std::cout << nPacketCounter << ": ";
    std::cout << "\033[m";

    if (bIncommingConnection) {
      std::cout << (bAccept ? "\033[32m" : "\033[91m");
    } else {
      std::cout << (bAccept ? "\033[92m" : "\033[91m");
    }
    std::cout << uRead << "b";
    if (bIncommingConnection) {
        std::cout << (bAccept ? " <---- " : " <-X-- ");
    } else {
        std::cout << (bAccept ? " ----> " : " --X-> ");
    }
    std::cout << "\033[0m";

    std::cout << (bIncommingConnection ? "\033[32m" : "\033[92m");
    std::cout << ipv4::NumberToAddress(bIncommingConnection ? packet.uSrc : packet.uDst);
    std::cout << "\033[0m";

    std::cout << " " << packet.sProtocol;
    std::cout << " ";
}

void PrintRule(const CRule &rule) {
    std::cout << "#" << rule.uNr << ": ";
    std::cout << ipv4::NumberToAddress(rule.uAddress) << "/" << +rule.uMaskValue;
    std::cout << " [" << rule.sProtocol << "] ";
    // std::cout << " (" << (uint32_t)rule.eRuleType << ")";
    std::cout << " #" << rule.sNote;
    std::cout << " ";
}

void PrintRules(const std::vector<CRule> &arRules) {
    std::cout << "\033[96m" << "---------------------------------------------------------------------" << "\033[0m" << std::endl;
    std::cout << "\033[96m";
    std::cout << "Rules loaded:" << std::endl;
    for (const CRule &rule : arRules) {
        PrintRule(rule);
        std::cout << std::endl;
    }
    std::cout << "\033[0m";
    std::cout << "\033[96m" << "---------------------------------------------------------------------" << "\033[0m" << std::endl;
}
void PrintAppliedRule(const CRule &rule, bool bIncommingConnection) {
    std::cout << (bIncommingConnection ? "\033[36m" : "\033[96m");
    PrintRule(rule);
    std::cout << "\033[0m";
    std::cout << " ";
}

void PrintTrackingDetails(const CRule &rule, const CRuleTrack &track, bool bIncommingConnection) {
    switch (rule.eRuleType) {
    case EFilterRule::LimitTime:
        std::cout << (track.bAccept ? "\033[92mACCEPT\033[0m" : "\033[91mDROP\033[0m");
        if (bIncommingConnection) {
            const std::time_t tmNow  = std::time(nullptr);
            const std::time_t tmSpan = tmNow - track.uValue;
            std::cout << "\033[93m ["
                      << internal::GetFormatedTime(tmSpan)
                      << "/"
                      << internal::GetFormatedTime(rule.uValue)
                      << "]"
                      << "\033[0m";
        }
        break;
    case EFilterRule::LimitDownload:
        std::cout << (track.bAccept ? "\033[92mACCEPT\033[0m" : "\033[91mDROP\033[0m");
        if (bIncommingConnection) {
            std::cout << "\033[32m ["
                      << internal::GetFormatedBytes(track.uValue, rule.cUnit)
                      << "/"
                      << internal::GetFormatedBytes(rule.uValue, rule.cUnit)
                      << ((rule.cUnit == 'b') ? "" : (std::string() + rule.cUnit))
                      << "b]"
                      << "\033[0m";
        }
        break;
    case EFilterRule::Undefined:
    default:
        std::cout << "ERROR: Internal error, unknown rule type" << std::endl;
        exit(-4);
        break;
    }
    std::cout << " ";
}
