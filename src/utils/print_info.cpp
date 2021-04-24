#include "print_info.h"

#include "print_info.h"
#include "track.h"
#include "ipv4_util.h"
#include "Rules.h"

void PrintAppTitle(){
    std::cout << "\033[1;33m" << "=====================================================================" << "\033[0m" << std::endl;
    std::cout << "\033[1;33m" << "***                        Tunnel Vission                         ***" << "\033[0m" << std::endl;
    std::cout << "\033[1;33m" << "=====================================================================" << "\033[0m" << std::endl;
}

void PrintTunnel(const char *sEthName, const char *sTunName1, const char *sTunName2){
    std::cout << "\033[93m" << "-----------------------------------------------------------------------" << "\033[0m" << std::endl;
    std::cout << "\033[93m" << "Tunnel created:" << "\033[0m" << std::endl;
    std::cout << "\033[93mAPP <--> [" << sTunName1 << "] <==TunVis==> [" << sTunName2 << "] <--> [" << sEthName << "] <--> INTERNET\033[0m" << std::endl;
    std::cout << "\033[93m" << "-----------------------------------------------------------------------" << "\033[0m" << std::endl;
}

void PrintCurrentDateTime() {
    std::time_t t = std::time(nullptr);
    char sTime[100];
    if (std::strftime(sTime, sizeof(sTime), "%Y-%m-%d %H:%M:%S", std::localtime(&t))) {
        std::cout << "\033[90m" << sTime << "\033[0m";
    }
    std::cout << " ";
}

void PrintOutgoingPacket(int64_t nPacketCounter, int16_t uRead, const CIpv4Packet &packet, bool bTerminate, bool bIncommingConnection) {
    std::cout << " [" << packet.sProtocol << "] ";
    // std::cout << "\033[0m";
}

void PrintTraffic(int64_t nPacketCounter, int16_t uRead, const CIpv4Packet &packet, bool bTerminate, bool bIncommingConnection) {
    std::cout << (bIncommingConnection ? "\033[32m" : "\033[92m");
    std::cout << nPacketCounter << ": ";
    std::cout << "\033[m";

    if (bIncommingConnection) {
      std::cout << (bTerminate ? "\033[91m" : "\033[32m");
    } else {
      std::cout << (bTerminate ? "\033[91m" : "\033[92m");
    }
    std::cout << uRead << "b";
    if (bIncommingConnection) {
        std::cout << (bTerminate ? " <-x-- " : " <---- ");
    } else {
        std::cout << (bTerminate ? " --x-> " : " ----> ");
    }
    std::cout << "\033[0m";

    std::cout << (bIncommingConnection ? "\033[32m" : "\033[92m");
    std::cout << ipv4::numberToAddress(bIncommingConnection ? packet.uSrc : packet.uDst);
    std::cout << "\033[0m";

    std::cout << " " << packet.sProtocol;
    std::cout << " ";
}

void PrintRule(const CFilterRule &rule) {
    std::cout << "#" << rule.uNr << ": ";
    // std::cout << rule.sTitle;
    std::cout << ipv4::numberToAddress(rule.uAddress) << "/" << +rule.uMaskValue;
    std::cout << " [" << rule.sProtocol << "] ";
    std::cout << " (" << (uint32_t)rule.eRuleType << ")";
    std::cout << " #" << rule.sNote;
    std::cout << " ";
}

void PrintRules(const std::vector<CFilterRule> &arRules) {
    std::cout << "\033[1;96m" << "-------------------------------------------------------" << "\033[0m" << std::endl;
    std::cout << "\033[96m";
    std::cout << "Rules loaded:" << std::endl;
    for (const CFilterRule &rule : arRules) {
        PrintRule(rule);
        std::cout << std::endl;
    }
    std::cout << "\033[0m";
    std::cout << "\033[1;96m" << "-------------------------------------------------------" << "\033[0m" << std::endl;
}
void PrintAppliedRule(const CFilterRule &rule, bool bIncommingConnection) {
    std::cout << (bIncommingConnection ? "\033[36m" : "\033[96m");
    PrintRule(rule);
    std::cout << "\033[0m";
    std::cout << " ";
}

constexpr const char *c_sByteUnits = "kmgt"; //kb, mb, gb, tb
// static inline std::string GetHumanReadableBytes(uint64_t uBytes) {
//     std::stringstream ss;
//     ss << std::fixed << std::setprecision(2);
//     if (uBytes < 1024) {
//         ss << uBytes << "b";
//         return ss.str();
//     }
//     double dHuman = uBytes;
//     for (const char *pUnit = c_sByteUnits; pUnit; ++pUnit) {
//         dHuman /= 1024.0;
//         if (dHuman < 10) {
//             ss << dHuman << *pUnit << "b";
//             return ss.str();
//         }
//     }
//     ss << "Invalid";
//     return ss.str();
// }

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

void PrintTrackingDetails(const CFilterRule &rule, const CRuleTrack &track, std::time_t now, bool bIncommingConnection) {
    switch (rule.eRuleType) {
    case EFilterRule::LimitTime:
        std::cout << (track.bTerminate ? "\033[91mTERM\033[0m" : "\033[92mOK\033[0m");
        if (bIncommingConnection) {
            std::cout << "\033[93m [" << (now - track.uValue) << " s]\033[0m";
        }
        break;
    case EFilterRule::LimitDownload:
        std::cout << (track.bTerminate ? "\033[91mTERM\033[0m" : "\033[92mOK\033[0m");
        if (bIncommingConnection) {
            std::cout << "\033[32m ["
                      << ::GetFormatedBytes(track.uValue, rule.cUnit)
                      << "/"
                      << ::GetFormatedBytes(rule.uValue, rule.cUnit)
                      << ((rule.cUnit == 'b') ? "" : (std::string() + rule.cUnit))
                      << "b"
                      << "]\033[0m";
        }
        break;
    case EFilterRule::Undefined:
    default:
        // std::cout << "ERROR: Internal error, unknown rule type" << std::endl;
        break;
    }
    std::cout << " ";
}
