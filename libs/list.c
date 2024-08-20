
#include "list.h"
#include "listnode.h"

/*
 *  ListAlloc() - 
 *
 *  Allocate a 'LIST' structure.
 *
 *  @PoolType: The type of memory pool to allocate from 
 *  (paged or nonpaged).
 *
 *  Return:
 *    - Pointer to the allocated 'LIST' structure on success.
 *    - 'NULL' if memory allocation fails.
 */
static PLIST ListAlloc(_PoolType_ POOL_TYPE PoolType) {

    PLIST List;

    List = ExAllocatePool2(PoolType, sizeof *List, 'list');
    if(!List) {
        return NULL;
    }

    RtlZeroMemory(List, sizeof *List);

    /*
     *  Store the pool type used for the allocation in 'List' to ensure 
     *  correct memory handling, regardless of whether the caller routine is 
     *  paged or nonpaged.
     */
    List->PoolType = PoolType;

    return List;
}

/*
 *  ListInit() -
 *
 *  Initializes a list structure with a specified maximum size and a custom
 *  free function. This function sets up the list's head, spinlock, and assigns
 *  the maximum number of elements and the function to be used for freeing
 *  elements.
 *
 *  @List: A pointer to the list structure to be initialized.
 *  @Max: The maximum number of elements the list can hold.
 *  @Free: A function pointer for the custom free function used to release
 *  elements.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the list is successfully initialized.
 *    - An appropriate 'NTSTATUS' error code if initialization fails.
 */
static NTSTATUS ListInit(_Inout_ PLIST List, _In_ SIZE_T Max, _In_ FREE Free) {

    Assert(List);

    if(Free) {
        List->Max  = Max;
        List->Free = Free;

        InitializeListHead(&List->Head);
        KeInitializeSpinLock(&List->Lock);
        return STATUS_SUCCESS;
    }

    return STATUS_UNSUCCESSFUL;
}

/*
 *  ListCreate() -
 *
 *  Allocates and initializes a list structure with the specified pool type,
 *  maximum size, and custom free function. If allocation or initialization
 *  fails, the list is destroyed.
 *
 *  @PoolType: The pool type from which memory is allocated for the list
 *  structure.
 *
 *  @Max: The maximum number of elements the list can hold.
 *  @Free: A function pointer for the custom free function used to release
 *  elements.
 *
 *  Return:
 *    - A pointer to the initialized list structure if successful.
 *    - 'NULL' if the list allocation or initialization fails.
 */
PLIST ListCreate(_PoolType_ POOL_TYPE PoolType, _In_ SIZE_T Max, _In_ FREE Free) {

    PLIST List;

    List = ListAlloc(PoolType);     
    if(List) {
        Status = ListInit(Max, Free);
        if(!NT_SUCCESS(Status)) {
            ListDestroy(&List);
        }
    }

    return List;
}

/*
 *  ListInsertNode() -
 *
 *  Inserts a node into the list while managing the list's size and concurrency.
 *  This function adds the specified 'ListNode' to the head of the list. If the
 *  list is at its maximum capacity ('List->Max'), it removes the oldest node
 *  (the tail) before inserting the new node to maintain the size constraint.
 *  
 *  The function currently returns 'STATUS_SUCCESS' in all cases, as the insertion
 *  process does not have any conditions that would result in failure. However,
 *  in future implementations, the return status could be expanded to reflect 
 *  different outcomes, such as failures in memory allocation or spinlock issues.
 *
 *  @List: A pointer to the 'LIST' structure representing the list into which
 *  the node is to be inserted.
 *
 *  @ListNode: A pointer to the 'LIST_NODE' structure representing the node to
 *  be inserted into the list.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' is always returned in the current implementation.
 */
static NTSTATUS ListInsertNode(_Inout_ PLIST List, _Inout_ PLIST_NODE ListNode) {

    KIRQL Irql;
    PVOID Data;
    PLIST_NODE Removed;

    Assert(List);
    Assert(ListNode);

    KeAcquireSpinLock(&List->Lock, &Irql);

    if(List->Items + 1 >= List->Max) {
        Removed = ListRemove(List);
        if(Removed) {
            Data = ListNodeDestroy(&Removed);
            List->Free(&Data);
        }
    } else {
        List->Items++;
    }

    InsertHeadList(&List->Head, &ListNode->Entry);
    KeReleaseSpinLock(&List->Lock, Irql);

    return STATUS_SUCCESS;
}

/*
 *  ListInsert() -
 *
 *  Inserts the provided data at the head of the linked list. This function 
 *  creates a new list node with the given data and inserts it at the beginning 
 *  of the list. If the list has reached its maximum capacity, the oldest node 
 *  is removed before the new node is inserted.
 *
 *  @List: A pointer to the `LIST` structure representing the list where the
 *  data is to be inserted.
 *
 *  @Data: A pointer to the data to be inserted into the list. This data is
 *  encapsulated in a `LIST_NODE` structure.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the data is successfully inserted into the list.
 *    - 'STATUS_UNSUCCESSFUL' if the node creation fails or the insertion
 *      process encounters an error.
 */
NTSTATUS ListInsert(_Inout_ PLIST List, _Inout_ PVOID Data) {

    NTSTATUS Status;
    PLIST_NODE ListNode;

    Assert(List);
    Assert(Data);

    Status   = STATUS_UNSUCCESSFUL;
    ListNode = ListNodeCreate(List->PoolType, Data);
    if(ListNode) {
        Status = ListInsertNode(List, ListNode);
    }

    return Status;
}

/*
 *  ListRemoveTailNode() -
 *
 *  Removes and returns the tail node from the linked list. The function locks
 *  the list, safely removes the tail node, updates the item count, and then
 *  unlocks the list.
 *
 *  @List: A pointer to the list from which the tail node will be removed.
 *
 *  Return:
 *    - A pointer to the removed list node if the operation is successful.
 *    - 'NULL' if the list is empty or if the removal fails.
 */
static PLIST_NODE ListRemoveTailNode(_Inout_ PLIST List) {

    KIRQL Irql;
    PLIST_NODE ListNode;
    PLIST_ENTRY Entry;

    ListNode = NULL;
    KeAcquireSpinLock(&List->Lock, &Irql);
    if(List->Items) {
        List->Items--;
        Entry = RemoveTailList(*List->Head);
        if(Entry) {
            Node = CONTAINING_RECORD(Entry, LIST_NODE, Entry);
        }
    }

    KeReleaseSpinLock(&List->Lock, Irql);

    return ListNode;
}

/*
 *  ListRemove() -
 *
 *  Removes the oldest node (the tail) from the linked list and returns the
 *  data stored in that node.
 *
 *  @List: A pointer to the list from which the oldest node will be removed.
 *
 *  Return:
 *    - A pointer to the data that was stored in the removed list node.
 *    - 'NULL' if the list is empty or if the removal fails.
 */
void* ListRemove(_Inout_ PLIST List) {

    PVOID Data;
    PLIST_NODE ListNode;

    Assert(List);

    Data = NULL;
    ListNode = ListRemoveTailNode(List);
    if(ListNode) {
        Data = ListNodeDestroy(&ListNode);
    }

    return Data;
}

/*
 *  ListDestroyNodes() -
 *
 *  Destroys all nodes in the linked list by repeatedly removing each node and 
 *  freeing its associated data. This function loops through the list, removing 
 *  the tail node using 'ListRemove()' and freeing the node's data using the 
 *  provided 'Free' function pointer until the list is empty.
 *
 *  @List: A pointer to the list whose nodes will be destroyed. The list itself 
 *  is not freed by this function, only the nodes within it.
 */
static inline void ListDestroyNodes(_Inout_ PLIST List) {

    while(List->Items) {
        List->Free(
            &ListRemove(List);
        );
    }
}

/*
 *  ListDestroy() -
 *
 *  Destroys the linked list and deallocates the memory associated with it.
 *  This function first removes and frees all nodes within the list by calling
 *  'ListDestroyNodes()'. After all nodes are removed, the function frees the
 *  list structure itself and sets the list pointer to 'NULL' to prevent any
 *  further access to the deallocated memory.
 *
 *  @List: A double pointer to the list that will be destroyed. The 
 *  pointer is set to 'NULL' after the list is destroyed.
 */
void ListDestroy(_Inout_ PLIST* List) {

    if(List && *List) {
        ListDestroyNodes(*List);
        ExFreePoolWithTag(*List, 'list');
        *List = NULL;
    }
}
