#include "tun.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <stdarg.h>

int InitializeTUN(const char *name, int flags) {
  const int fd = open("/dev/net/tun", O_RDWR);
  if (fd < 0) {
    perror("Opening /dev/net/tun");
    return fd;
  }

  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = flags;
  strncpy(ifr.ifr_name, name, IFNAMSIZ);

  const int err = ioctl(fd, TUNSETIFF, (void *)&ifr);
  if (err < 0) {
    perror("ioctl(TUNSETIFF)");
    close(fd);
    return err;
  }

  return fd;
}

/**************************************************************************
 * cread: read routine that checks for errors and exits if an error is    *
 *        returned.                                                       *
 **************************************************************************/
int cread(int fd, char *buf, int n){

  const int nread = read(fd, buf, n);
  if (nread < 0) {
    perror("Reading data");
    exit(1);
  }
  return nread;
}

/**************************************************************************
 * cwrite: write routine that checks for errors and exits if an error is  *
 *         returned.                                                      *
 **************************************************************************/
int cwrite(int fd, char *buf, int n){

  int nwrite;

  if((nwrite=write(fd, buf, n)) < 0){
    perror("Writing data");
    exit(1);
  }
  return nwrite;
}

/**************************************************************************
 * read_n: ensures we read exactly n bytes, and puts them into "buf".     *
 *         (unless EOF, of course)                                        *
 **************************************************************************/
int read_n(int fd, char *buf, int n) {
  int left = n;
  while (left > 0) {
    const int nread = cread(fd, buf, left);
    if (nread == 0) {
      return 0;
    }
    left -= nread;
    buf += nread;
  }
  return n;
}

//https://www.kernel.org/doc/Documentation/networking/tuntap.txt
int tun_alloc_mq(char *dev, int queues, int *fds)
  {
      struct ifreq ifr;
      int fd, err, i;

      if (!dev)
          return -1;

      memset(&ifr, 0, sizeof(ifr));
      /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
       *        IFF_TAP   - TAP device
       *
       *        IFF_NO_PI - Do not provide packet information
       *        IFF_MULTI_QUEUE - Create a queue of multiqueue device
       */
      ifr.ifr_flags = IFF_TAP | IFF_NO_PI | IFF_MULTI_QUEUE;
      strcpy(ifr.ifr_name, dev);

      for (i = 0; i < queues; i++) {
          if ((fd = open("/dev/net/tun", O_RDWR)) < 0)
             goto err;
          err = ioctl(fd, TUNSETIFF, (void *)&ifr);
          if (err) {
             close(fd);
             goto err;
          }
          fds[i] = fd;
      }

      return 0;
  err:
      for (--i; i >= 0; i--)
          close(fds[i]);
      return err;
  }
