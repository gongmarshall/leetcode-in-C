#include <stdio.h>
#include <stdlib.h>

struct test {
	unsigned int *trustip;
	int ip_num;
	unsigned int *blk_url;
	int url_num;
};

int main()
{
 int i = 0;
 struct test blkinfo;
 printf("size:%ld\n", sizeof(blkinfo));
 blkinfo.trustip = (unsigned int*)malloc(3*sizeof(unsigned int));
 blkinfo.blk_url = (unsigned int*)malloc(3*sizeof(unsigned int));
 printf("have malloc\n");
 for(i=0; i<3; i++)
 {
	printf("start %d\n", i);
 	blkinfo.trustip[i] = i*2;
	blkinfo.blk_url[i] = i*2 +1;
 }
//blkinfo->trustip = Trustip;
//blkinfo->blk_url = Blk_url;
 printf("size:%ld\n", sizeof(blkinfo));
 for(i=0; i<3; i++)
 {
	printf("start %d\n", i);
 	printf("trustip%d:%d\n",i,blkinfo.trustip[i]);
	printf("blk_url%d:%d", i,blkinfo.blk_url[i]);
 }
//blkinfo->trustip = Trustip;
 return 0;
}
