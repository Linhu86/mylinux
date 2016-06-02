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

void read_data(int fd)
{
    while(1)
    {
       if(read(fd, data_buf, 1) < 0)
          break;
       else
          printf("read from global fifo: %s.\n", data_buf);
    }
}

int main()
{
  fd = open(DEVICE_NAME, O_RDWR, S_IRUSR | S_IWUSR);
  if (fd < 0) {
    printf("device open failure\n");
  }

  fd_set rfds;

  struct timeval tv;

  int retval;

  FD_ZERO(&rfds);
  FD_SET(fd, &rfds);

  tv.tv_sec = 1;
  tv.tv_usec = 0;

  while(1)
  {

  retval = select(fd+1, &rfds, NULL, NULL, &tv);

  if(retval == -1)
  {
    perror("select()");
    break;
  }
  else if(retval)
  {
    printf("Data is available to read.\n");
    read_data(fd);
  }
  else
  {
    printf("No data present within 1s\n");
  }
  sleep(1);
  }

  return 0;  
}

