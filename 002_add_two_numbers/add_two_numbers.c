#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct ListNode {
	int val;
	struct ListNode *next;
};
struct ListNode* addTwoNumbers(struct ListNode* l1, struct ListNode* l2)
{
	int c_flag = 0;
	struct ListNode *p, *q;
	p = l1;
	q = l2;
	int i,j,val;
	struct ListNode* head=NULL;
	struct ListNode* tail=NULL;
	int flag = 0;
	while(p != NULL | q != NULL| c_flag != 0)
	{
		i=j=0;
		val=0;
		if(p != NULL)
		{
			i=p->val;
			p=p->next;
		}
		if(q != NULL)
		{
			j=q->val;
			q=q->next;
		}
		val = i+j+c_flag;
		if(val >= 10)
		{
			val -= 10;
			c_flag = 1;
		}
		else
			c_flag = 0;
		struct ListNode* lnode = malloc(sizeof(struct ListNode));
		lnode->val = val;
		lnode->next = NULL;
		if(flag == 0)
		{
			head = lnode;
			flag = 1;
		}
		if(tail != NULL)
		{
			tail->next = lnode;
		}
		tail = lnode;
	}
	return head;
}

static struct ListNode *node_build(const char *digits) 
{                                                      
	    struct ListNode *res, *p, *prev;                   
	        int first = 1;                                     
		    int len = strlen(digits);                          
		        const char *c = digits + len - 1;                  
			    prev = NULL;                                       
			        while (len-- > 0) {                                
					        p = malloc(sizeof(*p));                        
						        if (first) {                                   
								            first = 0;                                 
									                res = p;                                   
											        }                                              
							        p->val = *c-- - '0';                           
								        p->next = NULL;                                
									        if (prev != NULL) {                            
											            prev->next = p;                            
												            }                                              
										        prev = p;                                      
											    }                                                  
				                                                       
				    return res;                                        
}                                                      
                                                       
static void show(struct ListNode *ln)                  
{                                                      
	    int sum = 0, factor = 1;                           
	        while (ln != NULL) {                               
			        sum += ln->val * factor;                       
				        factor *= 10;                                  
					        ln = ln->next;                                 
						    }                                                  
		    printf("%d\n", sum);                               
}                                                      
                                                       
int main(int argc, char **argv)                        
{                                                      
	    if (argc < 3) {                                    
		            fprintf(stderr, "Usage: ./test n1 n2\n");      
			            exit(-1);                                      
				        }                                                  
	                                                           
	        struct ListNode *l1 = node_build(argv[1]);         
		    struct ListNode *l2 = node_build(argv[2]);         
		        struct ListNode *res = addTwoNumbers(l1, l2);      
			    show(l1);                                          
			        show(l2);                                          
				show(res);
				return 0;
}
