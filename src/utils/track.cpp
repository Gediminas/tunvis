#include "track.h"
#include "Rules.h"
#include "ipv4_util.h"

bool CheckRuleForTerm(const CFilterRule &rule, const CIpv4Packet &packet, CRuleTrack &track, uint16_t uRead) {
    const std::time_t now = std::time(nullptr);

    std::cout << "                        Read:"
              << uRead
              << " IHL:"
              << packet.uIHL*4
              << " HLen:"
              << packet.uHeaderLength
              << std::endl;

    switch (rule.eRuleType) {
    case EFilterRule::LimitTime:
        if (track.uValue == 0U) {
            track.uValue = now;
        } else if (now - track.uValue > rule.uValue) {
            return true;
        }
        break;
    case EFilterRule::LimitDownload:
        if (track.uValue < rule.uValue) {
            track.uValue += uRead;
            track.uValue -= packet.uIHL*4;
        } else {
            return true;
        }
        break;
    case EFilterRule::Undefined:
    default:
        // std::cout << "ERROR: Internal error, unknown rule type" << std::endl;
        break;
    }
    return false;
}
