#include "IPv4.h"
#include "Rules.h"
#include "Tracking.h"

void UpdateTracking(const CFilterRule &rule, const CIpv4Packet &packet, CRuleTrack &track, uint16_t uRead) {
    const std::time_t now = std::time(nullptr);

    switch (rule.eRuleType) {
    case EFilterRule::LimitTime:
        if (track.uValue == 0U) {
            track.uValue = now;
        } else if (now - track.uValue > rule.uValue) {
            track.bTerminate = true;
        }
        break;
    case EFilterRule::LimitDownload:
        if (track.uValue < rule.uValue) {
            track.uValue += uRead;
            track.uValue -= packet.uIHL*4;
            track.uValue -= 32;
        } else {
            track.bTerminate = true;
        }
        break;
    case EFilterRule::Undefined:
    default:
        // std::cout << "ERROR: Internal error, unknown rule type" << std::endl;
        break;
    }
}
