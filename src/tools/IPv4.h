#pragma once

#include <bits/stdc++.h>

enum class EProtocol : uint8_t {
    ANY    =  0xFF,
    HOPOPT =  0,
    ICMP   =  1,
    TCP    =  6,
    UDP    = 17,
};

class CIpv4Packet final {
 public:
    CIpv4Packet()  {}
    ~CIpv4Packet() {}
 public:
    uint8_t     uVersion     {0}; //  4 bits
    uint8_t     uIHL         {0}; //  4 bits
    uint8_t     uType        {0}; //  8 bits
    uint16_t    uTotalLength {0}; // 16 bits
    EProtocol   eProtocol    {EProtocol::ANY};
    std::string sProtocol;
    uint32_t    uSrc         {0};
    uint32_t    uDst         {0};
    std::string sSrc;
    std::string sDst;
};

class CTCPPacket final {
 public:
    CTCPPacket()  {}
    ~CTCPPacket() {}
 public:
    uint8_t uDataOffset {0}; // 4 bits
};

namespace ipv4 {
  CIpv4Packet ParseIpv4PacketHeader(const char *data, uint16_t uLength);
  CTCPPacket ParseTCPPacketHeader(const char *data, uint16_t uLength);

  uint32_t AddressToNumber(uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4);
  std::string NumberToAddress(const uint32_t uAddress);

  std::string ProtocolToStr(EProtocol eProtocol);
  EProtocol StrToProtocol(const char* sProtocol);
};
