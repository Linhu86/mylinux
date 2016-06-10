#include <sys/types.h>  
#include <sys/stat.h>  
#include <stdio.h>  
#include <fcntl.h>  
#include <unistd.h>  
#include <sys/ioctl.h>  
  
#define TANGLINUX _IO('T', 1)  
  
int main(void)  
{  
    int fd;  
  
    fd = open("/dev/tanglinux", O_RDWR);  
    if (fd < 0)  
    {  
    printf("can't open /dev/tanglinux\n");  
    return -1;  
    }  
  
    ioctl(fd, TANGLINUX);  
      
    return 0;  
}

