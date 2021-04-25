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
};

void UpdateTracking(const CFilterRule &rule, const CIpv4Packet &packet, CRuleTrack &track, char *buffer, uint16_t uRead);
