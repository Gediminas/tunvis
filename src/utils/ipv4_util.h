#pragma once

#include <bits/stdc++.h>

// https://www.techrepublic.com/article/exploring-the-anatomy-of-a-data-packet/

enum class EProtocol : uint8_t {
    ANY    =  0xFF,
    HOPOPT =  0,
    ICMP   =  1,
    TCP    =  6,
    UDP    = 17,
};

class CInfo final {
 public:
    CInfo()  {}
    ~CInfo() {}
 public:
    uint8_t  uVersion      {0}; //  4 bits
    uint8_t  uIHL          {0}; //  4 bits
    uint8_t  uType         {0}; //  8 bits
    uint16_t uHeaderLength {0}; // 16 bits

    EProtocol   eProtocol;
    std::string sProtocol;

    uint32_t    uSrc {0};
    uint32_t    uDst {0};

    std::string sSrc;
    std::string sDst;
};

namespace ipv4 {
  std::string numberToAddress(const uint32_t uAddress);
  uint32_t addressToNumber(uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4);
  CInfo parseIpv4(const char *data);

  EProtocol StrToProtocol(const char* sProtocol);
  std::string ProtocolToStr(EProtocol eProtocol);
};
