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

  try {
    while(getline(fs, sLine)) {
      std::cout << "\n[" << sLine << "]: " << std::endl;

      std::regex rgx("\\s+");
      std::sregex_token_iterator it(sLine.begin(), sLine.end(), rgx, -1);
      std::sregex_token_iterator end;
      int i = 0;
      for ( ; it != end; ++it) {
        std::cout << i++ << ": " << *it << '\n';
      }
    }
  } catch (std::regex_error& e) {
    // Syntax error in the regular expression
  }


  std::vector<CFilterRule> arRules;
  return arRules;
}
