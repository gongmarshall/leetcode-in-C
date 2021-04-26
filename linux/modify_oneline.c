#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
 
#define BUFF_LEN 256 
 
/*
 * 删除文件中无效内容
 * */
int clean_invalue_info(FILE *file_fd)
{
	assert(file_fd);
 
	char buff[BUFF_LEN] = {0};
	int buff_len = 0;
 
	long offset_read = 0;
	long offset_write = 0;
 
	while(fgets(buff, BUFF_LEN, file_fd)) {
        buff_len = strlen(buff);
 
        if (strstr(buff, "yzh")) { //有效数据要考虑移动
			if (offset_read == offset_write) {
				offset_write = offset_write + buff_len;
		        offset_read = offset_read + buff_len;
				continue;
			} else {
			    /*移动数据*/
			    fseek(file_fd, offset_write, SEEK_SET);
			    fputs(buff, file_fd);
			    offset_write = offset_write + buff_len;
 
		        offset_read = offset_read + buff_len;
		        fseek(file_fd, offset_read, SEEK_SET);
				continue;
			}
		}
		offset_read = offset_read + buff_len;
	}
	ftruncate(fileno(file_fd),offset_write);
}
 
int testt(FILE *file_fd)
{
	char buff[BUFF_LEN] = {0};
	int buff_len = 0;
	long offset_read = 0;
	char *str = "11111111111";
 
	fseek(file_fd, 0, SEEK_SET);
	fputs(str, file_fd);
	fgets(buff, BUFF_LEN, file_fd);
	printf("1. %s\n", buff);
 
	buff_len = strlen(buff);
	fgets(buff, BUFF_LEN, file_fd);
	printf("2. %s\n", buff);
 
	fseek(file_fd, buff_len, SEEK_SET);
	fgets(buff, BUFF_LEN, file_fd);
	printf("3. %s\n", buff);
 
	return 0;
}
 
 
int main(int argc, char *argv[])
{
	FILE *file_fd = fopen("./tt", "r+");
	clean_invalue_info(file_fd);
//    testt(file_fd);
	fclose(file_fd);
 
	return 0;
}
