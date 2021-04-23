#pragma once

#include <bits/stdc++.h>

class CFilterRule;
enum class EFilterRule;

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

bool CheckRuleForTerm(const CFilterRule &rule, CRuleTrack &track, uint16_t uRead);
