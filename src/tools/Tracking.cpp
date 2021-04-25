#include "IPv4.h"
#include "Rules.h"
#include "Tracking.h"

void UpdateTracking(const CFilterRule &rule, const CIpv4Packet &packet, CRuleTrack &track, char *buffer, uint16_t uRead) {
    if (track.bTerminate) {
        return;
    }
    switch (rule.eRuleType) {
    case EFilterRule::LimitTime: {
            const std::time_t now = std::time(nullptr);
            if (track.uValue == 0U) {
                track.uValue = now;
            } else if (now - track.uValue > rule.uValue) {
                track.bTerminate = true;
            }
        }
        break;
    case EFilterRule::LimitDownload:
        if (track.uValue < rule.uValue) {
            track.uValue += uRead;

            const uint16_t uIPHeaderSize = packet.uIHL * 4;
            track.uValue -= uIPHeaderSize;

            if (packet.eProtocol == EProtocol::TCP) {
                const uint16_t  uTCPPacketSize = uRead - uIPHeaderSize;
                const char     *tcp_buffer     = buffer + uIPHeaderSize;
                CTCPPacket tcp;
                tcp = ipv4::ParseTCPPacketHeader(tcp_buffer, uTCPPacketSize);

                const uint16_t uTCPHeaderSize = tcp.uDataOffset * 4;
                track.uValue -= uTCPHeaderSize;
            }
        } else {
            track.bTerminate = true;
        }
        break;
    case EFilterRule::Undefined:
    default:
        // std::cout << "ERROR: Internal error, unknown rule type" << std::endl;
        break;
    }
}
