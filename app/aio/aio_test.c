#include <sys/types.h>
#include <linux/aio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

const int SIZE_TO_READ = 100;

int main()
{
	// open the file
	int file = open("tmp.txt", O_RDONLY, 0);
	
	if (file == -1)
	{
		printf("Unable to open file!");
		return 1;
	}
	
	// create the buffer
	char* buffer = (char *)malloc(SIZE_TO_READ*sizeof(char));
	
	// create the control block structure
	struct aiocb cb;
	
	memset(&cb, 0, sizeof(aiocb));
	cb.aio_nbytes = SIZE_TO_READ;
	cb.aio_fildes = file;
	cb.aio_offset = 0;
	cb.aio_buf = buffer;
	
	// read!
	if (aio_read(&cb) == -1)
	{
		printf("Unable to create request!\n");
		close(file);
	}
	
	printf("Request enqueued!\n");
	
	// wait until the request has finished
	while(aio_error(&cb) == EINPROGRESS)
	{
		printf("Working...\n");
	}
	
	// success?
	int numBytes = aio_return(&cb);
	
	if (numBytes != -1)
		printf("Success!\n");
	else
		printf("Error!\n");
		
	// now clean up
	free(buffer);
	close(file);
	
	return 0;
}

