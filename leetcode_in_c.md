leetcode in C

# 1. 两数之和

> 给定一个整数数组 nums 和一个目标值 target，请你在该数组中找出和为目标值的那 两个 整数，并返回他们的数组下标。
>
> 你可以假设每种输入只会对应一个答案。但是，数组中同一个元素不能使用两遍。
>
>  
>
> 示例:
>
> 给定 nums = [2, 7, 11, 15], target = 9
>
> 因为 nums[0] + nums[1] = 2 + 7 = 9
> 所以返回 [0, 1]
>
> 来源：力扣（LeetCode）
> 链接：https://leetcode-cn.com/problems/two-sum
> 著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。

* 分析

  显然，主要是判断（target - nums[i]）是否存在数组中，一般是通过hash表来判断一个数值是否存在一个数组中。

  1. 定义一个hash表，并将nums数组存储到hash表中。
  2. 遍历nums，看hash表中是否存在target-nums[i].

  但问题是数组中两个最大值和最小值的差值比较大时空间利用效率不高

  可以将数组排序后，游标一样，两侧往中移。

> qsort()函数的使用：
>
> int (*cmp)(const void *, const void *);
>
> void qsort(void *base, size_t nmemb, size_t size, int(\*compar)(const void *, const void *));
>
> 参数说明：
>
> base:要排序的数组
>
> nmemb:数组中元素的数目
>
> size:每个数组元素占用内存空间，可使用sizeof获得
>
> compar：比较数组元素的比较函数。本比较函数的第一个参数值小于、等于、或大于第二参数值时,本比较函数的返回值应分别小于、等于、大于零。

```c
struct object {
    int val;
    int index;
};

static int compare(const void *a, const void *b)
{
    return ((struct object *) a)->val - ((struct object *) b)->val;
}

static int * twoSum(int *nums, int numsSize, int target, int* returnSize)
{
    int i, j;
    *returnSize = 0;
    struct object *objs = malloc(numsSize * sizeof(*objs));
    for (i = 0; i < numsSize; i++) {
        objs[i].val = nums[i];
        objs[i].index = i;
    }
    qsort(objs, numsSize, sizeof(*objs), compare);

    int count = 0;
    int *results = malloc(2 * sizeof(int));
    i = 0;
    j = numsSize - 1;
    while (i < j) {
        int diff = target - objs[i].val;
        if (diff > objs[j].val) {
            while (++i < j && objs[i].val == objs[i - 1].val) {}
        } else if (diff < objs[j].val) {
            while (--j > i && objs[j].val == objs[j + 1].val) {}
        } else {
            results[0] = objs[i].index;
            results[1] = objs[j].index;
            *returnSize = 2;
            return results;
        }
    }
    return NULL;
}
```

# 2. 两数相加

> 给出两个 非空 的链表用来表示两个非负的整数。其中，它们各自的位数是按照 逆序 的方式存储的，并且它们的每个节点只能存储 一位 数字。
>
> 如果，我们将这两个数相加起来，则会返回一个新的链表来表示它们的和。
>
> 您可以假设除了数字 0 之外，这两个数都不会以 0 开头。
>
> 示例：
>
> 输入：(2 -> 4 -> 3) + (5 -> 6 -> 4)
> 输出：7 -> 0 -> 8
> 原因：342 + 465 = 807
>
> 来源：力扣（LeetCode）
> 链接：https://leetcode-cn.com/problems/add-two-numbers
> 著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。

* 分析

  主要是遍历两个链表，当两个链表都不为NULL时，循环相加。在这个循环中每次判断两个链表的节点是否为NULL，然后相加并加上进位，最后将和与10比较，再赋值给进位和分配给新的节点。

  ```c
  /**
   * Definition for singly-linked list.
   * struct ListNode {
   *     int val;
   *     struct ListNode *next;
   * };
   */
  
  
  struct ListNode* addTwoNumbers(struct ListNode* l1, struct ListNode* l2){
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
  ```

  # 3. 无重复字符的最长子串

  > 给定一个字符串，请你找出其中不含有重复字符的 最长子串 的长度。
  >
  > 示例 1:
  >
  > 输入: "abcabcbb"
  > 输出: 3 
  > 解释: 因为无重复字符的最长子串是 "abc"，所以其长度为 3。
  >
  > 示例 2:
  >
  > 输入: "bbbbb"
  > 输出: 1
  > 解释: 因为无重复字符的最长子串是 "b"，所以其长度为 1。
  >
  > 示例 3:
  >
  > 输入: "pwwkew"
  > 输出: 3
  > 解释: 因为无重复字符的最长子串是 "wke"，所以其长度为 3。
  >      请注意，你的答案必须是 子串 的长度，"pwke" 是一个子序列，不是子串。
  >
  > 来源：力扣（LeetCode）
  > 链接：https://leetcode-cn.com/problems/longest-substring-without-repeating-characters
  > 著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。

* 分析

  因为是字符串，所以可以用一个128数组大小的hash表来储存字符，初始状态都设为0xff。

  然后再遍历字符串，对于hash表中为0xff的则是第一次出现，将该字符在字符串中的序号存储到该hash表中。长度自加。

  ​	而当hash表中不为0xff时，则此时该字符在字符串中的序号与之前储存的差值比较，若大于等于此时子串的长度则自加，否则子串长度为这个差值。

  比较子串长度获取最大值。

  ```c
  int lengthOfLongestSubstring(char * s){
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
  
  ```

  