#include <stdio.h>
#include <stdlib.h>

double findMedianSortedArrays(int* nums1, int nums1Size, int* nums2, int nums2Size){
	int n = nums1Size + nums2Size;
	int flag;
	int n1,n2;
	if(nums1Size == 0 && nums2Size == 0)
		return 0;
	if(nums1Size == 0)
	{
		if(nums2Size % 2 == 0)
		{
			n1 = nums2Size/2;
			n2 = n1 -1;
			return ((double)(nums2[n1]+nums2[n2])/2);
		}
		else
			return (double)(nums2[nums2Size/2]);
	}
	if(nums2Size == 0)
	{
		if(nums1Size % 2 == 0)
		{
			n1 = nums1Size/2;
			n2 = n1 -1;
		
			return ((double)(nums1[n1]+nums1[n2])/2);
		}
		else
			return ((double)nums1[nums1Size/2]);
	}
	if(n%2==0)
		flag = 1;
	else
	{
		flag = 0;
	}
	n /=2;
	n +=1;
	int i=0;
	int j=0;
	printf("n:%d\n",n);
	while(n--)
	{
		if(i==nums1Size)
		{
			n2=n1;
			n1=nums2[j];
			j++;
		}
		else if(j==nums2Size)
		{
			n2=n1;
			n1=nums1[i];
			i++;
		}
		else
		if(nums1[i]<nums2[j])
		{
			n2 = n1;
			n1 = nums1[i];
			i++;
		}
		else
		{
			n2 = n1;
			n1 = nums2[j];
			j++;
		}
		printf("n1:%d. n2%d\n",n1, n2);
	}
	if(flag)
		return ((double)(n1+n2)/2);
	else
			return ((double)(n1));



}

int main()
{
	int nums2[]= {1,3};
	int nums1[]= {2};
	printf("%f\n", findMedianSortedArrays(nums1, sizeof(nums1)/sizeof(*nums1), nums2, sizeof(nums2)/sizeof( *nums2)));
	return 0;
}
