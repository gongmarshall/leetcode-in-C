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
				printf("1 write:%ld, read:%ld \n", offset_write, offset_read);
				offset_write = offset_write + buff_len;
		        offset_read = offset_read + buff_len;
				continue;
			} else {
			    /*移动数据*/
				printf("2 write:%ld, read:%ld \n", offset_write, offset_read);
			    fseek(file_fd, offset_write, SEEK_SET);
			    fputs(buff, file_fd);
			    offset_write = offset_write + buff_len;
 
		        offset_read = offset_read + buff_len;
		        fseek(file_fd, offset_read, SEEK_SET);
				continue;
			}
		}
		printf("3 write:%ld, read:%ld \n", offset_write, offset_read);
		printf("read++\n");
		offset_read = offset_read + buff_len;
	}
	ftruncate(fileno(file_fd),offset_write);
}
/*这个函数是在没有匹配yzh时read_offet为读取到的这一行的末尾
 *而匹配到时，如果write_offset和read_offset相等代表不要我这一行去覆盖上一行
 *若write_offset和read_offset不相等则说明read_offset多加了一行没有包含关键字的行，需要把用本行来覆盖上一行(也可能是多行)。然后在定位到这行末尾。这样相当于，
 * */ 
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
