#pragma once

#include <bits/stdc++.h>

namespace tun {
  int InitializeTUN(const char *name, int flags);
  int cread(int fd, char *buf, int n);
  int cwrite(int fd, char *buf, int n);
  int read_n(int fd, char *buf, int n);
};
