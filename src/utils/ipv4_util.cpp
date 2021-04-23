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

std::string ipv4::ProtocolToStr(EProtocol eProtocol) {
    switch (eProtocol) {
    case EProtocol::ICMP: return "ICMP"; break;
    case EProtocol::TCP:  return "TCP";  break;
    case EProtocol::UDP:  return "UDP";  break;
    default: break;
    }
    return "";
}

EProtocol ipv4::StrToProtocol(const char* sProtocol) {
    if (strcmp(sProtocol, "ICMP") == 0) return EProtocol::ICMP;
    if (strcmp(sProtocol, "TCP") == 0)  return EProtocol::TCP;
    if (strcmp(sProtocol, "UDP") == 0)  return EProtocol::UDP;
    return EProtocol::ANY;
}

CInfo ipv4::parseIpv4(const char *data) {
    const uint32_t *pFirst = (uint32_t*) data;

    const uint32_t H0 = *(pFirst + 0);
    const uint32_t H2 = *(pFirst + 2);
    const uint32_t H3 = *(pFirst + 3);
    const uint32_t H4 = *(pFirst + 4);

    CInfo info;
    info.uVersion  = (H0 >> 4);
    info.eProtocol = (EProtocol)(uint8_t)(H2 >> 8);
    info.sProtocol = ipv4::ProtocolToStr(info.eProtocol);
    info.uSrc      = H3;
    info.uDst      = H4;
    info.sSrc      = ipv4::numberToAddress(info.uSrc);
    info.sDst      = ipv4::numberToAddress(info.uDst);
    return info;
}
