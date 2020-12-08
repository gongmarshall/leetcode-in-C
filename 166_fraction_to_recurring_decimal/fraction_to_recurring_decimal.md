* Description

  > 给定两个整数，分别表示分数的分子 numerator 和分母 denominator，以 字符串形式返回小数 。
  >
  > 如果小数部分为循环小数，则将循环的部分括在括号内。
  >
  > 如果存在多个答案，只需返回 任意一个 。
  >
  > 对于所有给定的输入，保证 答案字符串的长度小于 104 。
  >
  >  
  >
  > 示例 1：
  >
  > 输入：numerator = 1, denominator = 2
  > 输出："0.5"
  >
  > 示例 2：
  >
  > 输入：numerator = 2, denominator = 1
  > 输出："2"
  >
  > 示例 3：
  >
  > 输入：numerator = 2, denominator = 3
  > 输出："0.(6)"
  >
  > 示例 4：
  >
  > 输入：numerator = 4, denominator = 333
  > 输出："0.(012)"
  >
  > 示例 5：
  >
  > 输入：numerator = 1, denominator = 5
  > 输出："0.2"
  >
  >  
  >
  > 提示：
  >
  >     -231 <= numerator, denominator <= 231 - 1
  >     denominator != 0
  >
  > 通过次数16,655
  > 提交次数59,765
  >
  > 来源：力扣（LeetCode）
  > 链接：https://leetcode-cn.com/problems/fraction-to-recurring-decimal
  > 著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。

* Analysis

  对于循环小数的它的循环体的前一个余数是相同的，所以我首先将每次的余数存储到哈希链表中，而当下次的余数可以在哈希表中找到时，那么就可以确定循环体了。所以这个题最主要的就是哈希链表的使用。

  hash list in Linux kernel:

  ```c
  /*accord to member to get the head*/
  #define container_of(ptr,type, member) \
  		((type *)((char *)(ptr) - (size_t)&(((type *)0)->member)))
  #define list_entry(ptr, type, member) \
  		container_of(ptr, type, member)
  /**/
  #define hlist_for_each(pos, head) \
  	for(pos = (head)->first; pos; pos= pos->next)
  
  #define hlist_for_each_safe(pos, n, head) \
  	for(pos=(head)->first; pos&&({n=pos->next; true;}); pos = n)
  
  struct hlist_node {
      struct hlist_node *next, **pprev;
  };
  
  struct hlist_head {
      struct hlist_node *first;
  };
  
  static inline void INIT_HLIST_HEAD(struct hlist_head *h)
  {
      h->first = NULL;
  }
  
  static inline int hlist_empty(struct hlist_head *h)
  {
      return !h->first;
  }
  
  /*add a node to the tail of hash list*/
  static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
  {
      if(h->first != NULL){
          h->first->pprev = &n->next;
      }
      n->next = h->first;
      n->pprev = &h->first;
      n->first = n;
  }
  
  static inline void hlist_del(struct hlist_node *n)
  {
      struct hlist_node *next = n->next;
      struct hlist_node **pprev = n->pprev;
      *pprev = next;
      if(next != NULL){
          next->pprev = pprev;
      }
  }
  ```

  

* Solution

  1. inter = numerator/denominator; remainder = numerator%denominator

  2. 如果remainder等于0，直接转为字符串并退出。

  3. 如果remainder不等于0，在哈希链表中查找：

     i. 如果没有找到,将remainder存到哈希表中，保存remainder/denominator, remainder%=denominator; 进行第2步循环。

     ii. 如果找到，说明是循环体已找到 ，转换小数部分。