#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#define DEV_NAME "/dev/cdev"

static int fd;

char read_buf[1024];
char write_buf[1024];

void proc_read(void)
{
  printf("----------->proc_read Entry\n");

  memset(read_buf, 0, 1024);

  ssize_t count = read(fd, read_buf, 1024);

  if(count == 0)
    printf("No data read.\n");
  else
    printf("Read data %s\n", read_buf);
}


void proc_write(void)
{
  printf("----------->proc_write Entry\n");

  char *data = "012345678";

  ssize_t count = write(fd, data, strlen(data));

  if(count < 0)
  {
    printf("Failed to write data.\n");
  }
  else
  {
    printf("write %d data\n", count);
  }
}

int main()
{
  fd = open(DEV_NAME, O_RDWR);

  if(fd < 0)
	printf("open device failed %s\n", strerror(errno));

  pthread_t pthread_read;
  pthread_t pthread_write;

  if(pthread_create(&pthread_read, NULL, (void *)proc_read, NULL) < 0)
  {
    printf("Failed to create thread read.\n");
  }

  sleep(1);
  
  if(pthread_create(&pthread_write, NULL, (void *)proc_write, NULL) < 0)
  {
    printf("Failed to create thread write.\n");
  }

  pthread_detach(pthread_read);

  printf("read thread detached.\n");
  
  pthread_detach(pthread_write);

  while(1)
  {

  }
}



