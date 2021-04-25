#pragma once

#include <bits/stdc++.h>

class CRule;
class CRuleTrack;
class CIpv4Packet;

void PrintUsage(const std::string sAppName);
void PrintAppTitle();
void PrintTunnel(const char *sEthName, const char *sTunName1, const char *sTunName2);
void PrintCurrentDateTime();
void PrintOutgoingPacket(int64_t nPacketCounter, int16_t uRead, const CIpv4Packet &packet, bool bAccept, bool bIncommingConnection);
void PrintTraffic(int64_t nPacketCounter, int16_t uRead, const CIpv4Packet &packet, bool bAccept, bool bIncommingConnection);
void PrintRule(const CRule &rule);
void PrintRules(const std::vector<CRule> &arRules);
void PrintAppliedRule(const CRule &rule, bool bIncommingConnection);
void PrintTrackingDetails(const CRule &rule, const CRuleTrack &track, bool bIncommingConnection);
