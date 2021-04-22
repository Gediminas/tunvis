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
    uint32_t    uNr {0};
    uint32_t    uAddress {0};
    uint32_t    uMaskBits {0};
    int64_t     nRuleValue {0};
    std::string sRule;
    std::string sNote;
    EFilterRule eRule {EFilterRule::Undefined};
};

std::vector<CFilterRule> readRules(const char* sFileName) {
    std::vector<CFilterRule> arRules;
    std::fstream fs(sFileName, std::ios::in);
    std::string sLine, sCidr, sRule;
    uint32_t uNr = 0U;
    while (getline(fs, sLine)) {
        ++uNr;
        const std::vector<std::string> arsLinePart = explode(sLine, "#");
        if (!arsLinePart.size()) {
            continue;
        }
        if (arsLinePart.size() > 2) {
            std::cerr << "\033[1;31m" << "ERROR: Invalid line " << uNr << ": " << sLine << "\033[0m" << std::endl;
            continue;
        }
        const std::string sRulePart = arsLinePart[0];
        const std::vector<std::string> arsRulePart = explode(sRulePart, " ./");

        // for (const std::string &sToken : arsToken) {
        //     std::cout << sToken << std::endl;
        // }
        if (arsRulePart.size() != 6) {
            std::cerr << "\033[1;31m" << "ERROR: Invalid rule in line " << uNr << ": "  << sLine << "\033[0m" << std::endl;
            continue;
        }
        CFilterRule rule;
        const uint8_t a1 = stoi(arsRulePart[0]);
        const uint8_t a2 = stoi(arsRulePart[1]);
        const uint8_t a3 = stoi(arsRulePart[2]);
        const uint8_t a4 = stoi(arsRulePart[3]);

        const int8_t uMaskValue = stoi(arsRulePart[4]);
        if (uMaskValue > 32) {
            std::cerr << "Error in mask " << sLine << std::endl;
            continue;
        }

        rule.uNr         = uNr;
        rule.sTitle      = sLine;
        rule.uAddress    = addressToNumber(a1, a2, a3, a4);
        rule.uMaskBits   = uMaskValue ? (0xFFFFFFFF << (32 - uMaskValue)) : 0U;
        rule.sRule       = arsRulePart[5];
        rule.sNote       = arsLinePart.size() > 1 ? arsLinePart[1] : "";
        trim(rule.sNote);
        // rule.nRuleValue  = stoi(arsToken[5]);
        arRules.push_back(rule);
    }
    return arRules;
}
