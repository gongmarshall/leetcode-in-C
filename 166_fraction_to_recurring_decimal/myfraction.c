#include <stdio.h>
#include <stdlib.h>

/*根据结构体成员地址获取该结构体地址，已知该结构体成员的地址，减去该成员相对该结构体的偏移值*/
#define container_of(ptr, type, member) \
	((type *)((char *)(ptr) - (size_t)&(((type *)0)->member)))

/*list_entry为获取入口函数，即调用container_of*/
#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

/*获取每个哈希表的节点，为一个for循环*/
#define hlist_for_each(pos, head) \
	for(pos = (head)->first; pos; pos=pos->next)

/*hash表节点， @next: point to next node, @pprev: the address of previous node's next pointer*/

struct hlist_node  {
	struct hlist_node *next, **pprev;
};

/*hash talbe head pointer*/
struct hlist_head {
	struct hlist_node *first;
};

/*init the head of the hash list*/
static inline INIT_HLIST_HEAD(struct hlist_head *h)
{
	h->first = NULL;
}

/*add node from tail to head*/
static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
	struct hlist_node *first = h->first;
	n->next = first;
	if(first)
		first->pprev = &n->next;
	h->first = n;
	n->pprev = &h->first;
}

static inline void hlist_del(struct hlist_node *n)
{
	struct hlist_node *next = n->next;
	struct hlist_node **pprev = n->pprev;
	*pprev = next;
	if(next != NULL)
		next->pprev = pprev;
}
