#pragma once

#include <bits/stdc++.h>

namespace routing {
    void CreateTunnelRoutes(const char *sTunName1, const char *sTunName2, const char *sEthName, const char *sEthIP);
    void DestroyTunnelRoutes(const char *sTunName1, const char *sTunName2, const char *sEthName, const char *sEthIP);

    std::string GetDefaultEthName();
    std::string GetIPByDev(const char *sDev);
};
