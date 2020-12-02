#include <stdio.h>
#include <stdlib.h>

struct object {
	int index;
	int val;
};

static int compare(const void* a, const void* b)
{
	return ((((struct object *)a)->val)-(((struct object *)b)->val));
}

int *two_num(int* nums, int numsSize, int target, int* returnSize)
{
	int i;
	struct object* T_nums = malloc(numsSize*sizeof(T_nums));
	for(i=0; i<numsSize; i++)
	{
		T_nums[i].val = nums[i];
		T_nums[i].index = i;
	}
	qsort(T_nums, numsSize, sizeof(*T_nums), compare);
	
	int j;
	i=0;
	j = numsSize - 1;
	while(i<j)
	{
		int diff = target - T_nums[i].val;
		if(diff > T_nums[j].val)

			while(++i<j && T_nums[i].val == T_nums[i-1].val) {}
		else if(diff <T_nums[j].val)
			while(--j > i && T_nums[j].val == T_nums[j+1].val) {}
		else
		{
			returnSize[0] = T_nums[i].index;
			returnSize[1] = T_nums[j].index;
			return returnSize;
		}
	}
	return NULL;
}
int main(void)
{
	int nums[] = {2, 11, 5, 9, 15};
	int returnSize[2];
	int *p;
	p = two_num(nums, 5, 14, returnSize);
	if(p != NULL)
		printf("%d, %d\n", p[0], p[1]);
	else
		printf("not found\n");
	return 0;
}



