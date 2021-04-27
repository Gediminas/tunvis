#include "Rules.h"

#include "IPv4.h"
#include "StrUtil.h"

std::vector<CRule> filter_rules::readRules(const char* sFileName) {
    std::vector<CRule> arRules;
    std::fstream fs(sFileName, std::ios::in);
    if (!fs.is_open()) {
        std::cerr << "\033[91m" << "Rules file " << sFileName << " not found!" << "\033[0m" << std::endl;
        return arRules;
    }
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
            std::cerr << "\033[91m" << "ERROR: Invalid line " << uNr << ": " << sLine << "\033[0m" << std::endl;
            continue;
        }
        const std::string sRulePart = arsLinePart[0];
        const std::vector<std::string> arsRulePart = explode(sRulePart, " ./");
        if (arsRulePart.size() != 7) {
            std::cerr << "\033[91m" << "ERROR: Invalid rule in line " << uNr << ": "  << sLine << "\033[0m" << std::endl;
            continue;
        }

        CRule rule;
        const uint8_t a1 = std::stoi(arsRulePart[0]);
        const uint8_t a2 = std::stoi(arsRulePart[1]);
        const uint8_t a3 = std::stoi(arsRulePart[2]);
        const uint8_t a4 = std::stoi(arsRulePart[3]);

        const int8_t uMaskValue = std::stoi(arsRulePart[4]);
        if (uMaskValue > 32) {
            std::cerr << "\033[91m" << "ERROR: Invalid mask " << sLine << "\033[0m" << std::endl;
            continue;
        }

        rule.uNr         = uNr;
        rule.sTitle      = sLine;
        rule.uAddress    = ipv4::AddressToNumber(a1, a2, a3, a4);
        rule.uMaskValue  = uMaskValue;
        rule.uMaskBits   = uMaskValue ? (0xFFFFFFFF << (32 - uMaskValue)) : 0U;
        rule.sProtocol   = arsRulePart[5];
        rule.eProtocol   = ipv4::StrToProtocol(arsRulePart[5].c_str());
        rule.sNote       = arsLinePart.size() > 1 ? arsLinePart[1] : "";
        trim(rule.sNote);

        const auto [uValue, cUnit, eRuleType] = filter_rules::ParseRuleValueType(arsRulePart[6]);
        rule.uValue    = uValue;
        rule.cUnit     = cUnit;
        rule.eRuleType = eRuleType;

        arRules.push_back(rule);
    }
    return arRules;
}

int32_t filter_rules::findLastRule(const std::vector<CRule> &arRules, uint32_t uAddress, EProtocol eProtocol) {
    int32_t nIndex = -1;
    for (const CRule &rule : arRules) {
        ++nIndex;
        if ((uAddress & rule.uMaskBits) == (rule.uAddress & rule.uMaskBits) &&
            (eProtocol == rule.eProtocol || rule.eProtocol == EProtocol::ANY)) {
            return nIndex;
        }
    }
    return -1;
}

std::tuple<uint64_t, char, EFilterRule> filter_rules::ParseRuleValueType(const std::string &sText) {
    double       dValue = 0.0;
    char         cUnit  = ' ';
    EFilterRule  eType  = EFilterRule::Undefined;

    setlocale(LC_ALL,"");
    setlocale(LC_NUMERIC,"");

    const size_t i1 = sText.find_first_not_of("0123456789,.");
    if (i1 != std::string::npos) {
        std::string sValue = sText.substr(0, i1).c_str();
        std::replace(sValue.begin(), sValue.end(), ',', '.');
        dValue = std::stod(sValue);

        const std::string sUnits = sText.substr(i1, std::string::npos);
        if (sUnits == "b") {
            cUnit = 'b';
            eType = EFilterRule::LimitDownload;
        } else if (sUnits == "kb") {
            dValue *= 1024;
            cUnit = 'k';
            eType = EFilterRule::LimitDownload;
        } else if (sUnits == "mb") {
            dValue *= 1024;
            dValue *= 1024;
            cUnit = 'm';
            eType = EFilterRule::LimitDownload;
        } else if (sUnits == "gb") {
            dValue *= 1024;
            dValue *= 1024;
            dValue *= 1024;
            cUnit = 'g';
            eType = EFilterRule::LimitDownload;
        } else if (sUnits == "tb") {
            dValue *= 1024;
            dValue *= 1024;
            dValue *= 1024;
            dValue *= 1024;
            cUnit = 't';
            eType = EFilterRule::LimitDownload;
        } else if (sUnits == "s") {
            cUnit = 's';
            eType = EFilterRule::LimitTime;
        } else if (sUnits == "m") {
            cUnit = 'm';
            dValue *= 60;
            eType = EFilterRule::LimitTime;
        } else if (sUnits == "h") {
            cUnit = 'h';
            dValue *= 3600;
            eType = EFilterRule::LimitTime;
        }
    }
    const uint64_t uValue = std::round(dValue);
    return std::make_tuple(uValue, cUnit, eType);
}
