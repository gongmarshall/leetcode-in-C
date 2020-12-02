#include  <stdio.h>
#include <stdlib.h>
#include <string.h>
char * convert(char * s, int numRows){
	int len = strlen(s);
	int i,j;
	int cnt=0;
	int intval;
	/* This is critical step*/
	if(numsRows == 1 || numsRows >= len)
		return s;
	char *t = malloc((len+1)*sizeof(char));
	for(i=0;i<numRows; i++)
	{
		j=i;
		t[cnt++] = s[j];
		while(j<len)
		{
			intval= 2*(numRows-1-i);
			if(intval != 0)
			{
				j += intval;
			if(j<len)
				t[cnt++] = s[j];
			else
				break;
			}
			intval = 2*i;
			if(intval !=0)
			{
				j += intval;
				if(j<len)
					t[cnt++] = s[j];
				else
					break;
			}
		}
	}
	t[len] = '\0';
	return t;
}

int main()
{
	printf("%s\n", convert("LEETCODEISHIRING", 3));
	printf("%s\n", convert("LEETCODEISHIRING", 4));
	return 0;
}
