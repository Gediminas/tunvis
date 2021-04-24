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

        // for (const std::string &sPart : arsRulePart) {
        //     std::cout << sPart << std::endl;
        // }
        if (arsRulePart.size() != 7) {
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
        rule.uMaskValue  = uMaskValue;
        rule.uMaskBits   = uMaskValue ? (0xFFFFFFFF >> (32 - uMaskValue)) : 0U;
        rule.sProtocol   = arsRulePart[5];
        rule.eProtocol   = ipv4::StrToProtocol(arsRulePart[5].c_str());
        rule.sRule       = arsRulePart[6];
        rule.uRuleValue  = stoi(arsRulePart[6]);
        rule.sNote       = arsLinePart.size() > 1 ? arsLinePart[1] : "";
        trim(rule.sNote);

        rule.eRuleType   = EFilterRule::LimitDownload;
        // rule.eRuleType   = EFilterRule::LimitTime;

        arRules.push_back(rule);
    }
    return arRules;
}

// const CFilterRule* filter_rules::findLastRule(const std::vector<CFilterRule> &arRules, uint32_t uAddress) {
//     const CFilterRule* pRule = nullptr;
//     for (const CFilterRule &rule : arRules) {
//         if ((uAddress & rule.uMaskBits) == (rule.uAddress & rule.uMaskBits)) {
//             pRule = &rule;
//         }
//     }
//     return pRule;
// }

int32_t filter_rules::findLastRule(const std::vector<CFilterRule> &arRules, uint32_t uAddress, EProtocol eProtocol) {
    int32_t nIndex = -1;
    for (const CFilterRule &rule : arRules) {
        ++nIndex;
        if ((uAddress & rule.uMaskBits) == (rule.uAddress & rule.uMaskBits) &&
            (eProtocol == rule.eProtocol || rule.eProtocol == EProtocol::ANY)) {
            return nIndex;
        }
    }
    return -1;
}
