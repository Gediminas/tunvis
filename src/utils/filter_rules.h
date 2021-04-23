#pragma once

#include <bits/stdc++.h>

enum class EFilterRule {
    Undefined = 0,
    LimitTime,
    LimitDownload,
};

enum class EProtocol : uint8_t;

class CFilterRule final {
public:
    CFilterRule()  {}
    ~CFilterRule() {}
public:
    std::string sTitle;
    uint32_t    uNr {0};
    uint32_t    uAddress {0};
    uint32_t    uMaskBits {0};
    EProtocol   eProtocol;
    std::string sProtocol;
    uint64_t    uRuleValue {0};
    std::string sRule;
    std::string sNote;
    EFilterRule eRuleType {EFilterRule::Undefined};
};

namespace filter_rules {
    std::vector<CFilterRule> readRules(const char* sFileName);
    /* const CFilterRule* findLastRule(const std::vector<CFilterRule> &arRules, uint32_t uAddress); */
    int32_t findLastRule(const std::vector<CFilterRule> &arRules, uint32_t uAddress, EProtocol eProtocol);
    void displayRules(const std::vector<CFilterRule> &arRules);
};
