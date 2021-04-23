#pragma once

#include <bits/stdc++.h>

class CFilterRule;
class CRuleTrack;
class CInfo;

void print_current_time();
void PrintOutgoingPacket(int64_t nPacketCounter, int16_t uRead, const CInfo &info, bool bTerminate, bool bIncommingConnection);
void PrintTraffic(int64_t nPacketCounter, int16_t uRead, const CInfo &info, bool bTerminate, bool bIncommingConnection);
void PrintAppliedRule(const CFilterRule &rule, bool bIncommingConnection);
void PrintTrackingDetails(const CFilterRule &rule, const CRuleTrack &track, std::time_t now, bool bIncommingConnection);
