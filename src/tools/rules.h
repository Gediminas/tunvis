#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <regex>

class CFilterRule final {
public:
    CFilterRule()  {}
    ~CFilterRule() {}
public:
    uint32_t    uAddr {0};
    uint32_t    uMask {0};
};

std::vector<std::string> explode(const std::string &sText, const std::string &sDelims) {
    std::vector<std::string> arsTokens;
    size_t beg, pos = 0;
    while ((beg = sText.find_first_not_of(sDelims, pos)) != std::string::npos) {
        pos = sText.find_first_of(sDelims, beg + 1);
        arsTokens.push_back(sText.substr(beg, pos - beg));
    }
    return arsTokens;
}

std::vector<CFilterRule> readRules(const char* sFileName) {
    std::fstream fs(sFileName, std::ios::in);
    std::string sLine, sCidr, sRule;

    while(getline(fs, sLine)) {
        std::cout << "\n[" << sLine << "]: " << std::endl;

        const std::vector<std::string> arsToken = explode(sLine, " ./");

        for (const std::string &sToken : arsToken) {
            std::cout << sToken << std::endl;
        }

        if (arsToken.size()) {
            }
    }


    std::vector<CFilterRule> arRules;
    return arRules;
}
