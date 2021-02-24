#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
bool isPalindrome(int x)
{
	int num[32];
	int i=0;
	int j=0;
	if (x < 0 )
		return false;
	if (x == 0)
		return true;
	while(x != 0)
	{
		num[i++] = x % 10;
		x = x/10;
	}
	i=i-1;
	while(j<i)
	{
		if(num[j]!=num[i])
			return false;
		j++;
		i--;
	}
	return true;

}

int main()
{
	int ts1 = 0;
	int ts2 = 5;
	int ts3 = 151;
	int ts4 = -1;
	int ts5 = 123454321;
	int ts6 = 1234554321;
	printf("%d: %s\n", ts1, isPalindrome(ts1)?"true":"false");
	printf("%d: %s\n", ts2, isPalindrome(ts2)?"true":"false");
	printf("%d: %s\n", ts3, isPalindrome(ts3)?"true":"false");
	printf("%d: %s\n", ts4, isPalindrome(ts4)?"true":"false");
	printf("%d: %s\n", ts5, isPalindrome(ts5)?"true":"false");
	printf("%d: %s\n", ts6, isPalindrome(ts6)?"true":"false");
	return 0;
}
