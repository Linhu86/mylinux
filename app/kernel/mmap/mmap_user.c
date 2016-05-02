#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>

int main(){
  int fd;
  char *start;
  char *buf;
  char *end;
  char key_vals[20];

  printf("mypid is %d",getpid());
  fd = open("/dev/module",O_RDWR);
                                                                                                                                                                                                                                                                                                                                                                                                          
  buf = (char *)malloc(100);
  memset(buf, 0, 100);
  start=mmap(NULL,10,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
  end = mmap(NULL, 20,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);

  strcpy(buf,start);
  sleep (1);
  printf("buf 1 = %s\n",buf);

  strcpy(start,"Buf Is Not Null!rgrgrfgfgfdg");
  memset(buf, 0, 100);
  strcpy(buf,start);
  sleep (1);
  printf("buf 2 = %s\n",buf);

  strcpy(end,"is it reality! is not sure,are you ok, make sure ,you");
  memset(buf, 0, 100);
  strcpy(buf,start);
  sleep (1);
  printf("buf 3 = %s\n",buf);
  printf("buf 3 = %s\n",end);
                                                                                                                                                                                                                                                                                                                                                                                                   
  read(fd, key_vals, sizeof(key_vals));
  printf("key_vals 3 = %s\n",key_vals);
                                                                                                                                                                                                                                                                                                                                                                                                   
 // while(1);
  munmap(start,10);
  munmap(end,20);
                                                                                                                                                                                                                                                                                                                                                                                                   
  free(buf);
  close(fd);
  return 0;
}

