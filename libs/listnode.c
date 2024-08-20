
#include "listnode.h"

/*
 *  ListNodeAlloc() - 
 *
 *  Allocate a 'LIST_NODE' structure.
 *
 *  @PoolType: The type of memory pool to allocate from 
 *  (paged or nonpaged).
 *
 *  Return:
 *    - Pointer to the allocated 'LIST_NODE' structure on success.
 *    - 'NULL' if memory allocation fails.
 */
static PLIST_NODE ListNodeAlloc(_PoolType_ POOL_TYPE PoolType) {

    PLIST_NODE ListNode;

    ListNode = ExAllocatePool2(PoolType, sizeof *ListNode, 'lnde');
    if(!ListNode) {
        return NULL;
    }

    RtlZeroMemory(ListNode, sizeof *ListNode);

    /*
     *  Store the pool type used for the allocation in 'ListNode' to ensure 
     *  correct memory handling, regardless of whether the caller routine is 
     *  paged or nonpaged.
     */
    ListNode->PoolType = PoolType;

    return ListNode;
}

/*
 *  ListNodeInit() -
 *
 *  This function is intentionally simple, merely initializing a list node with
 *  the provided data. While the function could be considered unnecessary due
 *  to its simplicity, it is included for the sake of completeness and to adhere
 *  to the established coding standards within this codebase.
 *
 *  @ListNode: A pointer to the 'LIST_NODE' structure to be initialized.
 *  @Data: A pointer to the data that the list node will hold.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the node is successfully initialized.
 *    - 'STATUS_UNSUCCESSFUL' if the provided data is 'NULL'.
 */
static inline NTSTATUS ListNodeInit(_Inout_ PLIST_NODE ListNode, _In_ PVOID Data) {

    Assert(ListNode);

    if(Data) {
        ListNode->Data = Data;
        return STATUS_SUCCESS;
    }

    return STATUS_UNSUCCESSFUL;
}

/*
 *  ListNodeCreate() -
 *
 *  Allocates and initializes a new list node using the specified memory pool type.
 *  The function first allocates memory for the node and then initializes it with the
 *  provided data. If the initialization fails, the allocated memory is freed to
 *  avoid memory leaks.
 *
 *  @PoolType: The type of memory pool from which the node is allocated. This
 *  determines where the memory for the node will be allocated.
 *
 *  @Data: A pointer to the data that will be stored in the node. This data is
 *  associated with the node and will be accessible through it.
 *
 *  Return:
 *    - A pointer to the newly created list node if allocation and initialization
 *      are successful.
 *    - 'NULL' if either the allocation fails or the initialization fails.
 */
PLIST_NODE ListNodeCreate(_PoolType_ POOL_TYPE PoolType, _In_ PVOID Data) {

    NTSTATUS Status;
    PLIST_NODE ListNode;

    ListNode = ListNodeAlloc(PoolType);
    if(ListNode) {
        Status = ListNodeInit(ListNode, Data);
        if(!NT_SUCCESS(Status)) {
            ListNodeDestroy(&ListNode);
        }
    }

    return ListNode;
}

/*
 *  ListNodeDestroy() -
 *
 *  Frees the memory allocated for a list node and returns the data stored in
 *  the node. This function safely deallocates the node's memory, ensuring that
 *  the pointer is set to 'NULL' to avoid dangling pointers. It also returns the
 *  data that was stored in the node before destruction, allowing the caller to
 *  manage or free the data as needed.
 *
 *  @ListNode: A double pointer to the list node that is to be destroyed. The
 *  function checks if the node exists and then proceeds to free its memory and
 *  set the pointer to NULL.
 *
 *  Return:
 *    - A pointer to the data that was stored in the node before it was
 *      destroyed. If the node was not valid or already 'NULL', the function
 *      returns 'NULL'.
 */
void* ListNodeDestroy(_Inout_ PLIST_NODE* ListNode) {

    PVOID Data;

    Data = NULL;
    if(ListNode && *ListNode) {
        Data = (*ListNode)->Data;
        ExFreePoolWithTag((*ListNode), 'lnde');
        *ListNode = NULL;
    }
    
    return Data;
}
