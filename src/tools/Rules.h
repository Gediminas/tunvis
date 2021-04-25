#pragma once

#include <bits/stdc++.h>

enum class EProtocol : uint8_t;

enum class EFilterRule : uint8_t {
    Undefined = 0,
    LimitTime,
    LimitDownload,
};

class CRule final {
public:
    CRule()  {}
    ~CRule() {}
public:
    std::string sTitle;
    uint32_t    uNr        {0U};
    uint32_t    uAddress   {0U};
    uint8_t     uMaskValue {0U};
    uint32_t    uMaskBits  {0U};
    EProtocol   eProtocol;
    std::string sProtocol;
    uint64_t    uValue     {0U};
    char        cUnit;     // <space> or kmgt or hms
    std::string sNote;
    EFilterRule eRuleType {EFilterRule::Undefined};
};

namespace filter_rules {
    std::vector<CRule> readRules(const char* sFileName);
    int32_t findLastRule(const std::vector<CRule> &arRules, uint32_t uAddress, EProtocol eProtocol);
    std::tuple<uint64_t, char, EFilterRule> ParseRuleValueType(const std::string &sText);
};
