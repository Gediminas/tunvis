#include "print_info.h"

#include "print_info.h"
#include "track.h"
#include "ipv4_util.h"
#include "filter_rules.h"

void print_current_time() {
    std::time_t t = std::time(nullptr);
    char sTime[100];
    if (std::strftime(sTime, sizeof(sTime), "%Y-%m-%d %H:%M:%S", std::localtime(&t))) {
        std::cout << "\033[90m" << sTime << "\033[0m";
    }
}

void PrintOutgoingPacket(int64_t nPacketCounter, int16_t uRead, const CInfo &info, bool bTerminate, bool bIncommingConnection) {
    std::cout << " " << info.sProtocol;
    std::cout << "\033[0m";
}

void PrintTraffic(int64_t nPacketCounter, int16_t uRead, const CInfo &info, bool bTerminate, bool bIncommingConnection) {
    std::cout << (bIncommingConnection ? "\033[32m" : "\033[92m");
    std::cout << " " << nPacketCounter << ": ";
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
    std::cout << ipv4::numberToAddress(bIncommingConnection ? info.uSrc : info.uDst);
    std::cout << "\033[0m";

    std::cout << " " << info.sProtocol;
}

void PrintAppliedRule(const CFilterRule &rule, bool bIncommingConnection) {
    std::cout << (bIncommingConnection ? "\033[36m" : "\033[96m");
    std::cout << " => #" << rule.uNr <<  ": " << rule.sTitle;
    std::cout << "\033[0m";
}

void PrintTrackingDetails(const CFilterRule &rule, const CRuleTrack &track, std::time_t now, bool bIncommingConnection) {
    switch (rule.eRuleType) {
    case EFilterRule::LimitTime:
        std::cout << (track.bTerminate ? "\033[91m TERM\033[0m" : "\033[92m OK\033[0m");
        if (bIncommingConnection) {
            std::cout << "\033[93m [" << (now - track.uValue) << " s]\033[0m";
        }
        break;
    case EFilterRule::LimitDownload:
        std::cout << (track.bTerminate ? "\033[91m TERM\033[0m" : "\033[92m OK\033[0m");
        if (bIncommingConnection) {
            std::cout << "\033[95m [" << track.uValue << "b]\033[0m";
        }
        break;
    case EFilterRule::Undefined:
    default:
        // std::cout << "ERROR: Internal error, unknown rule type" << std::endl;
        break;
    }
}
