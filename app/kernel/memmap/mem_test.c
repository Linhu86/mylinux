#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h> 

#define PAGE_SIZE 4096


int main(int argc , char *argv[])
{
    int fd;
    int i;
    unsigned char *p_map;
    
    //\u6253\u5f00\u8bbe\u5907
    fd = open("/dev/mymap",O_RDWR);
    if(fd < 0)
    {
        printf("open fail\n");
        exit(1);
    }

    //\u5185\u5b58\u6620\u5c04
    p_map = (unsigned char *)mmap(0, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,fd, 0);
    if(p_map == MAP_FAILED)
    {
        printf("mmap fail\n");
        goto here;
    }

    //\u6253\u5370\u6620\u5c04\u540e\u7684\u5185\u5b58\u4e2d\u7684\u524d10\u4e2a\u5b57\u8282\u5185\u5bb9
    for(i=0;i<10;i++)
        printf("%d\n",p_map[i]);
    

here:
    munmap(p_map, PAGE_SIZE);
    return 0;
}

