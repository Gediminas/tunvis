#include "filter_rules.h"

#include "str_util.h"
#include "ipv4_util.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

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
        const uint8_t a1 = std::stoi(arsRulePart[0]);
        const uint8_t a2 = std::stoi(arsRulePart[1]);
        const uint8_t a3 = std::stoi(arsRulePart[2]);
        const uint8_t a4 = std::stoi(arsRulePart[3]);

        const int8_t uMaskValue = std::stoi(arsRulePart[4]);
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
        rule.sNote       = arsLinePart.size() > 1 ? arsLinePart[1] : "";
        trim(rule.sNote);

        const auto [ uValue, eRuleType ] = filter_rules::ParseRuleValueType(arsRulePart[6]);
        rule.uValue = uValue;
        rule.eRuleType = eRuleType;

        // rule.eRuleType   = EFilterRule::LimitDownload;
        // rule.eRuleType   = EFilterRule::LimitTime;
        arRules.push_back(rule);
    }
    return arRules;
}

std::pair<uint64_t, EFilterRule> filter_rules::ParseRuleValueType(const std::string &sText) {
        // const std::string sOrigValue = arsRulePart[6];
    double       dValue = 0.0;
    EFilterRule  eType  = EFilterRule::Undefined;

    setlocale(LC_ALL,"");
    setlocale(LC_NUMERIC,"");

    const size_t i1 = sText.find_first_not_of("0123456789,.");
    if (i1 != std::string::npos) {
        std::string sValue = sText.substr(0, i1).c_str();
        std::replace(sValue.begin(), sValue.end(), ',', '.');
        dValue = std::stod(sValue);

        const std::string sUnits = sText.substr(i1, std::string::npos);

        std::cout << ">>>> " << sValue << " " << dValue << " " << sUnits << std::endl;

        if (sUnits == "b") {
            eType = EFilterRule::LimitDownload;
        } else if (sUnits == "kb") {
            dValue *= 1024;
            eType = EFilterRule::LimitDownload;
        } else if (sUnits == "mb") {
            dValue *= 1024;
            dValue *= 1024;
            eType = EFilterRule::LimitDownload;
        } else if (sUnits == "gb") {
            dValue *= 1024;
            dValue *= 1024;
            dValue *= 1024;
            eType = EFilterRule::LimitDownload;
        } else if (sUnits == "tb") {
            dValue *= 1024;
            dValue *= 1024;
            dValue *= 1024;
            dValue *= 1024;
            eType = EFilterRule::LimitDownload;
        } else if (sUnits == "s") {
            eType = EFilterRule::LimitTime;
        } else if (sUnits == "m") {
            dValue *= 60;
            eType = EFilterRule::LimitTime;
        } else if (sUnits == "h") {
            dValue *= 3600;
            eType = EFilterRule::LimitTime;
        }
    }
    // const uint64_t uValue = (uint64_t)dValue;
    const uint64_t uValue = std::round(dValue);
    return std::make_pair(uValue, eType);
}

constexpr const char *c_sByteUnits = "kmgt"; //kb, mb, gb, tb

static std::string GetHumanReadableBytes(uint64_t uBytes) {
    std::stringstream ss;
    if (uBytes < 1024) {
        ss << uBytes << "b";
        return ss.str();
    }
    double dHuman = uBytes;
    for (const char *pUnit = c_sByteUnits; pUnit; ++pUnit) {
        dHuman /= 1024.0;
        if (dHuman < 10) {
            ss << dHuman << *pUnit << "b";
            return ss.str();
        }
    }
    ss << "Invalid";
    return ss.str();
}

std::string filter_rules::GetHumanRuleValue(const CFilterRule &rule) {
    switch (rule.eRuleType) {
    case EFilterRule::LimitDownload:
        ::GetHumanReadableBytes(rule.uValue);
        break;
    case EFilterRule::LimitTime:
        ::GetHumanReadableBytes(rule.uValue);
        break;
    default:
        break;
    }
    return "";
}

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
