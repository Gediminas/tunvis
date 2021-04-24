#pragma once

namespace routing {
    void CreateTunnelRoutes(const char *sEthName, const char *sTunName1, const char *sTunName2);
    void DestroyTunnelRoutes(const char *sEthName, const char *sTunName1, const char *sTunName2);
};
