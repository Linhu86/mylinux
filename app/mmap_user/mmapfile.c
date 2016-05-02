#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>

void printfMapChar(char *nameChar, char *mapChar)
{
  printf("%s = %s addr: %p\n\n", nameChar, mapChar, mapChar);
}

void printfMmapAddr(char *nameChar, char *mmapChar){//??mmapChar???
  printf("%s'address = %p\n",nameChar, mmapChar);
}


void printfDivLine(char *desc){
  printf("********%s*******\n", desc);
}


int main()
{
  int fd;
  char *mapChar;
  char *checkChar;
  char *ptr;
  printf("mypid is %d\n", getpid());

  fd = open("/tmp/test.txt", O_RDWR);

  mapChar = mmap(NULL, 1000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  
  printfDivLine("Print map memory zone mapChar");
  printfMapChar("mapChar", mapChar);

  ptr = mapChar;

  memcpy(ptr, "abcdefg", 8);

  printfDivLine("Print map memory zone mapChar after modification");
  printfMapChar("mapChar", mapChar);

/*
  strcpy(mapChar, "writeSrc,writeSrc,writeSrc,writeSrc,writeSrc,writeSrc,");
  printfDivLine("write data into memory map zone via mapChar.");
  printfMapChar("mapChar", mapChar);

  checkChar = mmap(NULL, 100, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
*/

  msync(mapChar, 8, MS_ASYNC);

  close(fd);
  
  //printfDivLine("checkChar check");
  printfMmapAddr("mapChar", mapChar);
  //printfMmapAddr("checkChar", checkChar);
  //printfMapChar("checkChar", checkChar);
  munmap(mapChar, 1000);
  //munmap(checkChar, 100);
  return 0;
}



