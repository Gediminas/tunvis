#pragma once

#include "helper.h"
#include "ipv4.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <regex>

enum class EFilterRule {
    Undefined = 0,
    LimitTime,
    LimitDownload,
};

class CFilterRule final {
public:
    CFilterRule()  {}
    ~CFilterRule() {}
public:
    std::string sTitle;
    uint32_t    uAddress {0};
    uint32_t    uMaskBits {0};
    int64_t     nRuleValue {0};
    std::string sRule;
    EFilterRule eRule {EFilterRule::Undefined};
};

std::vector<CFilterRule> readRules(const char* sFileName) {
    std::vector<CFilterRule> arRules;
    std::fstream fs(sFileName, std::ios::in);
    std::string sLine, sCidr, sRule;
    while(getline(fs, sLine)) {
        const std::vector<std::string> arsToken = explode(sLine, " ./");

        // for (const std::string &sToken : arsToken) {
        //     std::cout << sToken << std::endl;
        // }
        if (arsToken.size() != 6) {
            std::cerr << "Error in rule " << sLine << std::endl;
            continue;
        }
        CFilterRule rule;
        const uint8_t a1 = stoi(arsToken[0]);
        const uint8_t a2 = stoi(arsToken[1]);
        const uint8_t a3 = stoi(arsToken[2]);
        const uint8_t a4 = stoi(arsToken[3]);

        const int8_t uMaskValue = stoi(arsToken[4]);
        if (uMaskValue > 32) {
            std::cerr << "Error in mask " << sLine << std::endl;
            continue;
        }

        rule.sTitle      = sLine;
        rule.uAddress    = addressToNumber(a1, a2, a3, a4);
        rule.uMaskBits   = (uMaskValue < 32) ? (0xFFFFFFFF << (32 - uMaskValue)) : 0U;
        rule.sRule       = arsToken[5];
        // rule.nRuleValue  = stoi(arsToken[5]);
        arRules.push_back(rule);
    }
    return arRules;
}
