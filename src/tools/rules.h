#pragma once

#include <iostream>
#include <vector>
#include <fstream>

class CFilterRule final {
public:
  CFilterRule()  {}
  ~CFilterRule() {}
public:
  uint32_t    uAddr {0};
  uint32_t    uMask {0};
};

std::vector<CFilterRule> readRules(const char* sFileName) {
  std::fstream fs(sFileName);
  std::string sLine;
  while (fs >> sLine) {

    std::cout << sLine;

    fs >> sLine;
    std::cout << " | " << sLine << std::endl;
  }
  std::vector<CFilterRule> arRules;
  return arRules;
}
