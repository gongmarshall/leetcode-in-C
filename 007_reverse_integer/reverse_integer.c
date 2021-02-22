#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
		
int reverse(int x){
    unsigned int reverse = 0;
    int reseult;
    int flag = 1;
    int input = x;
    if(input < 0)
    {
        flag = -1;
        input = -input;
    }
    printf("%d,%d\n",x, input);
        while(input!=0)
        {
                if(reverse > INT_MAX)
                        return 0;
                reverse = reverse*10+ x%10;
                input = input/10;
        }
        reseult = ((int)reverse) * flag;
        return reverse;

}
int main()
{
	printf("main reverser:%d\n",reverse(-112));
	return 0;
}
