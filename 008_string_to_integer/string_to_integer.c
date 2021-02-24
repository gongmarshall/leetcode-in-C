#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
int parse_value(unsigned int result, int flag)
{
	unsigned int cmp;

			if (flag == 1)
			{
				if (result > INT_MAX)	
				{
					return INT_MAX;
				}
				else
					return (int)result;
			}
			else
			{
				cmp = INT_MAX+1;
				if(result > cmp)
					return INT_MIN;
				else
					return (int)(-result);
			}
								

}

int myAtoi(char* s)
{
	int flag = 1;
	unsigned int cmp;
	long result = 0;
	while(*s != '\0')
	{
		while(*s == ' ')
			s++;
		if(*s == '-')
		{
			flag = 0;
			s++;
		}
		else if(*s == '+')
			s++;
		while(*s != '\0')
		{
			if(*s < '0'|| *s > '9')
				return parse_value(result, flag);
			result = result*10 + (*s - '0');
			printf("debug:%ld\n", result);
			if (flag == 1)
			{
				if (result > INT_MAX)	
				{
					return INT_MAX;
				}
			}
			else
			{
				cmp = INT_MAX+1;
				if(result > cmp)
					return INT_MIN;
			}
			s++;
		}
	}
	return flag?(result):(-result);
								
}

int main()
{
//	char str[]="-345str";
//	char str[]="   12345";
	char str[]=" -+3472332";
	printf("%d\n", myAtoi(str));
	return 0;
}
