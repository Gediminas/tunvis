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

constexpr int BUFSIZE {2000}; //for reading from tun/tap interface, must be >= 1500
constexpr int PORT    {55555};
constexpr int debug   {1};
constexpr const char *if_name = "tunvis";
constexpr int flags = IFF_TUN | IFF_NO_PI; //IFF_TAP

int InitializeTUN(const char *name, int flags) {
  const char *clonedev = "/dev/net/tun";
  const int fd = open(clonedev, O_RDWR);
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

  int nread;

  if((nread=read(fd, buf, n)) < 0){
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

  int nread, left = n;

  while(left > 0) {
    if ((nread = cread(fd, buf, left)) == 0){
      return 0 ;
    }else {
      left -= nread;
      buf += nread;
    }
  }
  return n;
}

/**************************************************************************
 * do_debug: prints debugging stuff (doh!)                                *
 **************************************************************************/
void do_debug(const char *msg, ...){

  va_list argp;

  if (debug) {
    va_start(argp, msg);
    vfprintf(stderr, msg, argp);
    va_end(argp);
  }
}

/**************************************************************************
 * my_err: prints custom error messages on stderr.                        *
 **************************************************************************/
void my_err(const char *msg, ...) {

  va_list argp;

  va_start(argp, msg);
  vfprintf(stderr, msg, argp);
  va_end(argp);
}

int main() {

  std::cout << "Tunnel Vission!" << std::endl;

  const int tun_fd = InitializeTUN(if_name, flags);
  if (tun_fd < 0) {
    my_err("Error connecting to tun/tap interface %s!\n", if_name);
    exit(1);
  }
  std::cout << "OK" << std::endl;

  do_debug("Successfully connected to interface %s\n", if_name);

  int sock_fd;
  if ( (sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket()");
    exit(1);
  }

  /* Server, wait for connections */

  int maxfd;
  uint16_t nread, nwrite, plength;
  char buffer[BUFSIZE];
  struct sockaddr_in local, remote;
  unsigned short int port = PORT;
  int net_fd, optval = 1;
  socklen_t remotelen;
  unsigned long int tap2net = 0, net2tap = 0;

  /* avoid EADDRINUSE error on bind() */
  if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval)) < 0) {
    perror("setsockopt()");
    exit(1);
  }

  memset(&local, 0, sizeof(local));
  local.sin_family = AF_INET;
  local.sin_addr.s_addr = htonl(INADDR_ANY);
  local.sin_port = htons(port);
  if (bind(sock_fd, (struct sockaddr*) &local, sizeof(local)) < 0) {
    perror("bind()");
    exit(1);
  }

  if (listen(sock_fd, 5) < 0) {
    perror("listen()");
    exit(1);
  }

  /* wait for connection request */
  remotelen = sizeof(remote);
  memset(&remote, 0, remotelen);
  if ((net_fd = accept(sock_fd, (struct sockaddr*)&remote, &remotelen)) < 0) {
    perror("accept()");
    exit(1);
  }

  do_debug("SERVER: Client connected from %s\n", inet_ntoa(remote.sin_addr));

  /* use select() to handle two descriptors at once */
  maxfd = (tun_fd > net_fd)?tun_fd:net_fd;

  while(1) {
    fd_set rd_set;
    FD_ZERO(&rd_set);
    FD_SET(tun_fd, &rd_set); FD_SET(net_fd, &rd_set);

    int ret = select(maxfd + 1, &rd_set, NULL, NULL, NULL);

    if (ret < 0 && errno == EINTR){
      continue;
    }

    if (ret < 0) {
      perror("select()");
      exit(1);
    }

    if(FD_ISSET(tun_fd, &rd_set)) {
      /* data from tun/tap: just read it and write it to the network */

      nread = cread(tun_fd, buffer, BUFSIZE);

      tap2net++;
      do_debug("TAP2NET %lu: Read %d bytes from the tap interface\n", tap2net, nread);

      /* write length + packet */
      plength = htons(nread);
      nwrite = cwrite(net_fd, (char *)&plength, sizeof(plength));
      nwrite = cwrite(net_fd, buffer, nread);

      do_debug("TAP2NET %lu: Written %d bytes to the network\n", tap2net, nwrite);
    }

    if(FD_ISSET(net_fd, &rd_set)) {
      /* data from the network: read it, and write it to the tun/tap interface.
       * We need to read the length first, and then the packet */

      /* Read length */
      nread = read_n(net_fd, (char *)&plength, sizeof(plength));
      if(nread == 0) {
        /* ctrl-c at the other end */
        break;
      }

      net2tap++;

      /* read packet */
      nread = read_n(net_fd, buffer, ntohs(plength));
      do_debug("NET2TAP %lu: Read %d bytes from the network\n", net2tap, nread);

      /* now buffer[] contains a full packet or frame, write it into the tun/tap interface */
      nwrite = cwrite(tun_fd, buffer, nread);
      do_debug("NET2TAP %lu: Written %d bytes to the tap interface\n", net2tap, nwrite);
    }
  }

  return(0);
}
