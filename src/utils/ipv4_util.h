#pragma once

#include <bits/stdc++.h>

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

std::string numberToAddress(const uint32_t uAddress);
uint32_t addressToNumber(uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4);
CInfo parseIpv4(const char *data);
