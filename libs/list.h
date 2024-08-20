#ifndef LIST_H
#define LIST_H

#include "comm.h"

struct _LIST {

    POOL_TYPE PoolType;
    
    SIZE_T      Max;
    KSPIN_LOCK  Lock;
    LIST_ENTRY  Head;

    __volatile SIZE_T Items;
};

typedef struct _LIST LIST, *PLIST;

typedef void (*FREE)(void**)

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
extern PLIST
ListCreate(
        _PoolType_ POOL_TYPE PoolType,
        _In_       SIZE_T    Max,
        _In_       FREE      Free;
    );

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
extern NTSTATUS
ListInsert(
        _Inout_ PLIST List,     
        _Inout_ PVOID Data
    );

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
extern void*
ListRemove(
        _Inout_ PLIST List 
    );

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
extern void
ListDestroy(
        _Inout_ PLIST* List
    );

#endif  /* LIST_H */
