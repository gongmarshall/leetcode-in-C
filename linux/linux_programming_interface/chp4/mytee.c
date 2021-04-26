#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	int flag=0;
	int i;
	int file_flags=O_CREAT |O_WRONLY;
	char buffer[1024+1];
	int ch = getopt(argc, argv, "a");
	if (ch == 'a')
	{
		flag=1;
	}
	else
		/* remove the content of the file */
		file_flags |= O_TRUNC;
	int fd_in;
	int fd_out;
	int nums;
/*	fd_in=open(0, O_RDONLY);
	if (fd_in < =0 )
	{
		printf("open stdin error\n");
		return -1;
	}
	*/
	while((nums=read(STDIN_FILENO, buffer, 1024))>0)
	{
				if(write(STDOUT_FILENO, buffer, nums)!= nums)
				{
					fprintf(stderr, "write stdout error\n");
					return -3;
				}
	for(i=flag+1; i<argc; i++)
	{
			if((fd_out=open(argv[i], file_flags, S_IRUSR|S_IWUSR)) < 0)
			{
				printf("open %s error\n", argv[i]);
				return -2;
			}
			if(flag)
				/*next byte of the end of the file*/
				lseek(fd_out, 0, SEEK_END);
				if(write(fd_out, buffer, nums) < 0)
				{
					fprintf(stderr, "write file error\n");
					close(fd_out);
					return -2;
				}
			close(fd_out);

	}	
	}
	return 0;
}
