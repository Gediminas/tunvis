#include "track.h"
#include "filter_rules.h"

bool CheckRuleForTerm(const CFilterRule &rule, CRuleTrack &track, uint16_t uRead) {
    const std::time_t now = std::time(nullptr);

    switch (rule.eRuleType) {
    case EFilterRule::LimitTime:
        if (track.uValue == 0U) {
            track.uValue = now;
        } else if (now - track.uValue > rule.uValue) {
            return true;
        }
        break;
    case EFilterRule::LimitDownload:
        if (track.uValue + uRead <= rule.uValue) {
            track.uValue += uRead;
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
