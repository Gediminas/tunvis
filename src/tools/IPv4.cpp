#include "IPv4.h"

#include <arpa/inet.h>

CIpv4Packet ipv4::ParseIpv4PacketHeader(const char *data, uint16_t uLength) {
    const uint32_t *pFirst = (uint32_t*) data;
    const uint32_t H0 = ntohl(*(pFirst + 0));
    const uint32_t H2 = ntohl(*(pFirst + 2));
    const uint32_t H3 = ntohl(*(pFirst + 3));
    const uint32_t H4 = ntohl(*(pFirst + 4));

    CIpv4Packet packet;
    packet.uVersion     = (H0 & 0xf0000000) >> 28;  //  4 bits
    packet.uIHL         = (H0 & 0x0f000000) >> 24;  //  4 bits
    packet.uType        = (H0 & 0x00ff0000) >> 16;  //  8 bits
    packet.uTotalLength = (H0 & 0x0000ffff);        // 16 bits
    packet.eProtocol    = (EProtocol)(uint8_t)((H2 & 0x00ff0000) >> 16);
    packet.sProtocol    = ipv4::ProtocolToStr(packet.eProtocol);
    packet.uSrc         = H3;
    packet.uDst         = H4;
    packet.sSrc         = ipv4::NumberToAddress(packet.uSrc);
    packet.sDst         = ipv4::NumberToAddress(packet.uDst);

    if (packet.uTotalLength && packet.uTotalLength != uLength) {
        std::cerr << "\033[0;33m" << "WARNING: Invalid packet length: " << packet.uTotalLength << " != " << uLength << " (buffer)" << "\033[0m" << std::endl;
    }
    return packet;
}

CTCPPacket ipv4::ParseTCPPacketHeader(const char *data, uint16_t uLength) {
     const uint32_t *pFirst = (uint32_t*) data;
     const uint32_t H3 = ntohl(*(pFirst + 3));

    CTCPPacket packet;
    packet.uDataOffset = (H3 & 0xf0000000) >> 28;  //  4 bits

    return packet;
}
uint32_t ipv4::AddressToNumber(uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4) {
    return (a1 << 24) | (a2 << 16) | (a3 << 8) | a4;
}

std::string ipv4::NumberToAddress(const uint32_t uAddress) {
    const uint8_t a1 = (uint8_t) (uAddress >> 24);
    const uint8_t a2 = (uint8_t) (uAddress >> 16);
    const uint8_t a3 = (uint8_t) (uAddress >> 8);
    const uint8_t a4 = (uint8_t) (uAddress);
    std::stringstream ss;
    ss << +a1 << "." << +a2 << "." << +a3 << "." << +a4;
    return ss.str();
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
