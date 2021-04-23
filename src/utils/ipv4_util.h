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
    // 4 version
    // 4 IHL
    // 8 Type
    // 16 Leng
    //
    // 16 Id
    // 4 Flags
    // FragOffset
    //
    // 8 TTL
    // 8 Protocol
    // 16 HeaderChecksum
    // S
    // D
    // Options
    // Data

    uint16_t    uSize {0};

    //h0
    uint8_t     uVersion        {0}; //  4 bits
    /* uint8_t     uIHL            {0}; //  4 bits */
    /* uint8_t     uType           {0}; //  8 bits */
    /* uint16_t    uHeaderLength   {0}; // 16 bits */

    /* //h1 */
    /* uint16_t    uId             {0}; // 16 bits */
    /* uint8_t     uFlags          {0}; //  4 bits */

    //h2
    /* uint8_t     uTTL            {0}; //  8 bits */
    uint8_t     uProtocol       {0}; //  8 bits
    EProtocol   eProtocol;
    std::string sProtocol;
    /* uint16_t    uHeaderChecksum {0}; // 16 bits */

    //h3
    uint32_t    uSrc            {0}; // 32 bits
    std::string sSrc;

    //h4
    uint32_t    uDst            {0}; // 32 bits
    std::string sDst;

    //h5
    /* uint32_t    uOptions        {0}; // 32 bits */

    //DATA
};

namespace ipv4 {
  std::string numberToAddress(const uint32_t uAddress);
  uint32_t addressToNumber(uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4);
  CInfo parseIpv4(const char *data);

  EProtocol StrToProtocol(const char* sProtocol);
  std::string ProtocolToStr(EProtocol eProtocol);
};
