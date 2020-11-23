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

