#ifndef LIST_NODE_H
#define LIST_NODE_H

struct _LIST_NODE {
    
    PVOID      Data;
    LIST_ENTRY Entry;
};

typedef struct _LIST_NODE LIST_NODE, *PLIST_NODE;

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
extern PLIST_NODE 
ListNodeCreate(
        _PoolType_ POOL_TYPE PoolType,
        _In_ PVOID Data
    );

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
extern void*
ListNodeDestroy(
        _Inout_ PLIST_NODE* ListNode
    );

#endif  /* LIST_NODE_H */
