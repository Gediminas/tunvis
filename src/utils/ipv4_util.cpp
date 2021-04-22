#include "ipv4_util.h"

#include <sstream>

std::string ipv4::numberToAddress(const uint32_t uAddress) {
    const uint8_t a1 = (uint8_t) (uAddress);
    const uint8_t a2 = (uint8_t) (uAddress >> 8);
    const uint8_t a3 = (uint8_t) (uAddress >> 16);
    const uint8_t a4 = (uint8_t) (uAddress >> 24);
    std::stringstream ss;
    ss << +a1 << "." << +a2 << "." << +a3 << "." << +a4;
    return ss.str();
}

uint32_t ipv4::addressToNumber(uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4) {
    return (a4 << 24) | (a3 << 16) | (a2 << 8) | a1;
}

CInfo ipv4::parseIpv4(const char *data) {
    const uint32_t *pFirst = (uint32_t*) data;
    CInfo info;
    info.uSrc = *(pFirst + 3);
    info.uDst = *(pFirst + 4);
    return info;
}
