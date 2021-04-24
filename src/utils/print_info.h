#pragma once

#include <bits/stdc++.h>

class CFilterRule;
class CRuleTrack;
class CIpv4Packet;

void PrintAppTitle();
void PrintTunnel(const char *sEthName, const char *sTunName1, const char *sTunName2);
void PrintCurrentDateTime();
void PrintOutgoingPacket(int64_t nPacketCounter, int16_t uRead, const CIpv4Packet &packet, bool bTerminate, bool bIncommingConnection);
void PrintTraffic(int64_t nPacketCounter, int16_t uRead, const CIpv4Packet &packet, bool bTerminate, bool bIncommingConnection);
void PrintRule(const CFilterRule &rule);
void PrintRules(const std::vector<CFilterRule> &arRules);
void PrintAppliedRule(const CFilterRule &rule, bool bIncommingConnection);
void PrintTrackingDetails(const CFilterRule &rule, const CRuleTrack &track, std::time_t now, bool bIncommingConnection);
