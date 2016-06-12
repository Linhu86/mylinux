#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>

static int fd,len;
static char buf[64]={0};

void func(int signo)
{
	printf("signo %d \n",signo);
	read(fd,buf,64);
	printf("buf=%s \n",buf);	
}

int main()
{

	int flage,i=0;

	fd = open("/dev/hello",O_RDWR | O_ASYNC);
	if(fd<0)
	{
		perror("open fail \n");
		return ;
	}


	fcntl(fd,F_SETOWN,getpid());
	flage = fcntl(fd,F_GETFL);
	fcntl(fd,F_SETFL,flage|FASYNC);

	signal(SIGIO,func);

	while(1)
	{

		sleep(1);
		printf("%d\n",i++);
	}

	close(fd);
}

