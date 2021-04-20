#pragma once

#include <sstream>

class CInfo final {
public:
  CInfo()  {}
  ~CInfo() {}
public:
  uint32_t    uSrc {0};
  uint32_t    uDst {0};
  std::string sSrc;
  std::string sDst;
  uint16_t    uSize {0};
};

std::string toIpv4Address(const uint32_t uAddress) {
  const uint8_t a1 = (uint8_t) (0xFF &  uAddress);
  const uint8_t a2 = (uint8_t) (0xFF & (uAddress >> 8));
  const uint8_t a3 = (uint8_t) (0xFF & (uAddress >> 16));
  const uint8_t a4 = (uint8_t) (0xFF & (uAddress >> 24));
  std::stringstream ss;
  ss << +a1 << "." << +a2 << "." << +a3 << "." << +a4;
  return ss.str();
}

CInfo parseIpv4(const char *data) {
  const uint32_t *pFirst = (uint32_t*) data;
  CInfo info;
  info.uSrc = *(pFirst + 3);
  info.uDst = *(pFirst + 4);
  info.sSrc = toIpv4Address(info.uSrc);
  info.sDst = toIpv4Address(info.uDst);
  return info;
}
