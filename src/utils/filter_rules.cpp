#include "filter_rules.h"

#include "str_util.h"
#include "ipv4_util.h"

#include <iostream>
#include <fstream>

std::vector<CFilterRule> filter_rules::readRules(const char* sFileName) {
    std::vector<CFilterRule> arRules;
    std::fstream fs(sFileName, std::ios::in);
    std::string sLine, sCidr, sRule;
    uint32_t uNr = 0U;
    while (getline(fs, sLine)) {
        ++uNr;
        if (sLine.empty() || sLine[0] == '#') {
            continue;
        }
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
        rule.uAddress    = ipv4::addressToNumber(a1, a2, a3, a4);
        rule.uMaskBits   = uMaskValue ? (0xFFFFFFFF >> (32 - uMaskValue)) : 0U;
        rule.sRule       = arsRulePart[5];
        rule.sNote       = arsLinePart.size() > 1 ? arsLinePart[1] : "";
        trim(rule.sNote);
        // rule.nRuleValue  = stoi(arsToken[5]);
        arRules.push_back(rule);
    }
    return arRules;
}

const CFilterRule* filter_rules::findLastRule(const std::vector<CFilterRule> &arRules, uint32_t uAddress) {
    const CFilterRule* pRule = nullptr;
    for (const CFilterRule &rule : arRules) {
        if ((uAddress & rule.uMaskBits) == (rule.uAddress & rule.uMaskBits)) {
            pRule = &rule;
        }
    }
    return pRule;
}

void filter_rules::displayRules(const std::vector<CFilterRule> &arRules) {
    std::cout << "\033[1;96m" << "-------------------------------------------------------" << "\033[0m" << std::endl;
    std::cout << "\033[96m"  << "Rules loaded:" << "\033[0m" << std::endl;
    for (const CFilterRule &rule : arRules) {
        std::cout << "\033[96m"  << "#" << rule.uNr << ":   " << rule.sTitle << "\033[0m" << std::endl;
    }
    std::cout << "\033[1;96m" << "-------------------------------------------------------" << "\033[0m" << std::endl;
}
