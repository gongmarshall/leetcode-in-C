#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MIN(a, b) (a>b?b:a)
char * longestPalindrome(char * s){
	int len = strlen(s);
	int len_t = 2*len +1;
	char *t= malloc(len_t*sizeof(char));
	int *p= malloc(len_t*sizeof(int));
	memset(p, 0, len_t);
	int i,j;
	t[len_t-1]='\0';
	t[0] = '$';
	for(i=0; i<len; i++)
	{
		t[i<<1 +1] = '#';
		t[i<<1 +2]= s[i];
	}
	printf("%s\n", t);
	int mx = 0; //right edge
	int max = 0;
	int id = 0;
	int center = 0;
	for(i=0; i<len_t; i++)
	{
		p[i] = mx > i ? MIN(p[2*id-i], mx-i) : 1;
		while(t[i+p[i]] == t[i-p[i]]) ++p[i]; //caculate one by one
		if(mx < i + p[i])
		{
			mx = i +p[i];
			id = i;
		}
		if(max < p[i])
		{
			max = p[i];
			center = i;
		}
	}
	char *q = malloc(max*sizeof(char));
	i = (center-max)/2 ;
	printf("i:%d,max:%d \n", i, max);
	strncpy(q, s+i, max-1);
	q[max-1] = '\0';
	return q;
}

int main()
{
	printf("%s\n", longestPalindrome("12212"));
	printf("%s\n", longestPalindrome("122122"));
	printf("%s\n", longestPalindrome("waabwwfd"));
	printf("%s\n", longestPalindrome("abcbbcbd"));
}
