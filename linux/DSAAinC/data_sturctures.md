# LIST, STACK, QUEUES
## LIST
* basic node
```c
typedef int ElementType;
typedef struct Node *PtrToNode;
typedef PtrToNode List;
typedef PtrToNode Position;
struct Node
{
	ElementType Element;
	Position Next;
}

```

* functions
  * List MakeEmpty(List L)
  * int IsEmpty(List L)
  * IsLast(Position P, List L)
  * Position Find(ElementType X, List L)
  * Delete(ElementType X, List L)
  * Position FindPrevious(ElementType X, List L)
  * Insert(ElementType X, List L, Position P)
  * DeleteList(List L)
  * Position Header(List L)
  * Position First(List L)
  * Position Advance(Position P)
  * ElementType Retrieve(Position P)