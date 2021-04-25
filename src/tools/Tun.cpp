#include "Tun.h"

#include <fcntl.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>

constexpr int flags = IFF_TUN | IFF_NO_PI;

int tun::InitTun(const char *name) {
    const int fd = open("/dev/net/tun", O_RDWR);
    if (fd < 0) {
        std::cerr << "\033[91m" << "ERROR: Failed to connect TUN interface " << name << "!" << "\033[0m" << std::endl;
        exit(1);
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

int tun::Read(int fd, char *buffer, int n) {
    const int nRead = read(fd, buffer, n);
    if (nRead < 0) {
        perror("Reading data");
        exit(1);
    }
    return nRead;
}

int tun::Write(int fd, char *buffer, int n) {
    const int nWrite = write(fd, buffer, n);
    if (nWrite < 0){
        perror("Writing data");
        exit(1);
    }
    return nWrite;
}
