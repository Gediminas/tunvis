#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

#include <linux/if_tun.h>

#define max(a,b) ((a)>(b) ? (a):(b))

int main()
{
   const int f1 = open("/dev/tunvis1", O_RDWR);
   const int f2 = open("/dev/tunvis2", O_RDWR);
   const int fm = max(f1, f2) + 1;
   std::cout << f1 << f2 << fm << std::endl;

   ioctl(f1, TUNSETNOCSUM, 1);
   ioctl(f2, TUNSETNOCSUM, 1);

   char buf[1600];
   fd_set fds;

   while(1){
	FD_ZERO(&fds);
        FD_SET(f1, &fds);
        FD_SET(f2, &fds);

	select(fm, &fds, NULL, NULL, NULL);

	if( FD_ISSET(f1, &fds) ) {
	   l = read(f1,buf,sizeof(buf));
           write(f2,buf,l);
	}
	if( FD_ISSET(f2, &fds) ) {
	   l = read(f2,buf,sizeof(buf));
           write(f1,buf,l);
	}
   }
}
