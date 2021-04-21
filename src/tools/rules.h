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

std::vector<CFilterRule> readRules(const char* sFileName) {
    std::fstream fs(sFileName, std::ios::in);
    std::string sLine, sCidr, sRule;

    // while(fs >> sCidr) {
    //   std::cout << sCidr << std::endl;
    //   fs >> sRule;
    //   std::cout << sRule << std::endl;
    // }

    // while(getline(fs, sLine)) {
    //   std::cout << sLine << std::endl;
    //   std::regex sep ("[ /]");
    //   std::sregex_token_iterator tokens(sLine.cbegin(), sLine.cend(), sep, -1);
    //   std::sregex_token_iterator end;
    //   for(; tokens != end; ++tokens){
    //     std::cout << "token found: " << *tokens << std::endl;
    //   }

    //   // std::cout << sCidr << " | " << sRule << std::endl;
    // }

    std::string const delims{ " ./" };

    while(getline(fs, sLine)) {
        std::cout << "\n[" << sLine << "]: " << std::endl;

        size_t beg, pos = 0;
        while ((beg = sLine.find_first_not_of(delims, pos)) != std::string::npos) {
            pos = sLine.find_first_of(delims, beg + 1);
            std::cout << sLine.substr(beg, pos - beg) << std::endl;
        }
    }


    std::vector<CFilterRule> arRules;
    return arRules;
}
