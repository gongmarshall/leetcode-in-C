#include <stdio.h>
#include <stdlib.h>

int parse_value(int sv)
{

}

int myAtoi(char* s)
{
	unsigned int result = 0;
	while(*s != '\0')
	{
		while(*s == ' ')
			s++;
		if(*s == '-')
			flag = 0;
		if(*s < '0'|| *s > '9')
			return parse_value(result);
		result = result*10 + (*s - '0');
		if (result > INT_MAX)
}
