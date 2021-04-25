#pragma once

#include <bits/stdc++.h>

namespace tun {
  int InitTun(const char *name);
  int Read(int fd, char *buffer, int n);
  int Write(int fd, char *buffer, int n);
};
