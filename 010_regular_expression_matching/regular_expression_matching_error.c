#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
bool isMatch(char *s, char *p)
{
	char* q=p;
	char record;
	printf("s:%s, p:%s\n", s, p);
	while(*s != '\0' && *p != '\0')
	{
		printf("s:%c, p:%c\n", *s, *p);
		if(*p == '.')
		{
			s++;
			p++;
			printf("1 s:%c, p:%c\n", *s, *p);
		}
		else if(*p == '*')
		{
			printf("1\n");
			if(p == q)
			{
				printf("p++\n");
				p++;
			}
			else 
			{
				if(*(p-1) == '.')
				{
					printf("2\n");
					while(*s != '\0')
						s++;
				}
				else
				{
					printf("p-1:%c\n", *(p-1));
					while(*s == *(p-1))
						s++;
					record = *(p-1);
					p++;
					while(*p == record)
						p++;
					printf("p++ 2\n");
				}
			}
		}
		else
		{
			printf("3\n");
		       	if(*s != *p)
				if(*(p+1) == '*')
				{
					p++;
					p++;
				}
				else
					return false;
			else
			{
				s++;
				p++;
			}
			printf("3 s:%c, p:%c\n", *s, *p);	
		}
		printf("4\n");
	}
	
	printf("s: %c, p: %c\n", *s, *p);

	while(*p == '*')
		p++;
	if(*s != '\0' || *p != '\0')
		return false;
	else
		return true;
}
int main(int argc, char* argv[])
{
	printf("%s \n", isMatch(argv[1], argv[2])?"true":"false");
	return 0;
}
