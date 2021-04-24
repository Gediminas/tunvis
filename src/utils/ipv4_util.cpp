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

CIpv4Packet ipv4::parseIpv4Packet(const char *data) {
    const uint32_t *pFirst = (uint32_t*) data;

    const uint32_t H0 = *(pFirst + 0);
    const uint32_t H2 = *(pFirst + 2);
    const uint32_t H3 = *(pFirst + 3);
    const uint32_t H4 = *(pFirst + 4);

    CIpv4Packet packet;
    packet.uVersion      = (H0 & 0x000000f0) >> 4;   //  4 bits
    packet.uIHL          =  H0 & 0x0000000f;         //  4 bits
    packet.uType         = (H0 & 0x0000ff00) >> 8;   //  8 bits
    packet.uHeaderLength = (H0 & 0xffff0000) >> 16;  // 16 bits
    packet.eProtocol     = (EProtocol)(uint8_t)((H2 & 0x0000ff00) >> 8);
    packet.sProtocol     = ipv4::ProtocolToStr(packet.eProtocol);
    packet.uSrc          = H3;
    packet.uDst          = H4;
    packet.sSrc          = ipv4::numberToAddress(packet.uSrc);
    packet.sDst          = ipv4::numberToAddress(packet.uDst);
    return packet;
}
