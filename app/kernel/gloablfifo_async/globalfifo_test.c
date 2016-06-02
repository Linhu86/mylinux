/*
 * globalfifo\u5f02\u6b65\u901a\u77e5\u6d4b\u8bd5\u6587\u4ef6
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>

#define DEVICE_NAME "/dev/globalfifo"
int fd;
char data_buf[1];

void input_handler(int signum)
{
  while(1)
  {
     if(read(fd, data_buf, 1) < 0)
        break;
     else
        printf("read from global fifo: %s.\n", data_buf);
  }
  printf("receive a signal from globalfifo,signalnum:%d\n",signum);
}

int main()
{
  int oflags;
  fd = open(DEVICE_NAME, O_RDWR, S_IRUSR | S_IWUSR);
  if (fd !=  - 1) {
    signal(SIGIO, input_handler); 
    fcntl(fd, F_SETOWN, getpid());
    oflags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, oflags | FASYNC);
    while(1) {
      sleep(2);
    }
  } else {
    printf("device open failure\n");
  }
}

