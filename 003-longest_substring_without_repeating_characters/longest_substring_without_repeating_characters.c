#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int lengthOfLongestSubstring(char *s)
{
	int Shash[128];
	memset(Shash, -1, sizeof(Shash));
	int max = 0;
	if (s == NULL)
		return 0;
	int i;
	int len=0;
	for(i=0; s[i] != '\0'; i++)
	{
		if(Shash[s[i]] == -1)
			len++;
		else
		{
			if(i-Shash[s[i]]>len)
				len++;
			else
				len=i-Shash[s[i]];
		}
		Shash[s[i]] = i;
		if(max < len)
			max = len;
	}
	return max;
}

int main()
{
	char p[]="hijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789hijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789hijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789hijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789hijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789hijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	printf("%d\n",lengthOfLongestSubstring(p));
	return 0;
}
