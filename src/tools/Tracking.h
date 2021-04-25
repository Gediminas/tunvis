#pragma once

#include <bits/stdc++.h>

class CFilterRule;
class CIpv4Packet;
enum class EFilterRule : uint8_t;

class CRuleTrack {
public:
    CRuleTrack()  {}
    ~CRuleTrack() {}
public:
    EFilterRule eRule; // {EFilterRule::Undefined};
    uint64_t    uValue {0U};
    bool        bTerminate {false};
    // CFilterRule *pRule {nullptr};

    // std::string sTitle;
    // uint32_t    uNr {0};
    // uint32_t    uAddress {0};
    // uint32_t    uMaskBits {0};
    // int64_t     nRuleValue {0};
    // std::string sRule;
    // std::string sNote;
};

void UpdateTracking(const CFilterRule &rule, const CIpv4Packet &packet, CRuleTrack &track, uint16_t uRead);
