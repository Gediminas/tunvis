#pragma once

#include <bits/stdc++.h>

enum class EFilterRule : uint8_t {
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
    uint32_t    uNr        {0U};
    uint32_t    uAddress   {0U};
    uint8_t     uMaskValue {0U};
    uint32_t    uMaskBits  {0U};
    EProtocol   eProtocol;
    std::string sProtocol;
    uint64_t    uValue     {0U};
    std::string sNote;
    EFilterRule eRuleType {EFilterRule::Undefined};
};

namespace filter_rules {
    std::vector<CFilterRule> readRules(const char* sFileName);
    int32_t findLastRule(const std::vector<CFilterRule> &arRules, uint32_t uAddress, EProtocol eProtocol);

    std::pair<uint64_t, EFilterRule> ParseRuleValueType(const std::string &sText);
    std::string GetHumanRuleValue(const CFilterRule &rule);
};
