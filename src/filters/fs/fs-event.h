#ifndef FS_EVENT_H
#define FS_EVENT_H

#include "comm.h"

struct _FS_EVENT {

    POOL_TYPE   PoolType;
    PPROC       Proc;
    TIME        Time;
    UCHAR       MjFunc;
    ULONG       Options;
    PFILE_EVENT FileEvent;
};

typedef struct _FS_EVENT FS_EVENT, *PFS_EVENT;

/*
 *  FSEventCreate() -
 *
 *  Allocates and initializes a 'FS_EVENT' structure for file system 
 *  events. If initialization fails, the allocated 'FS_EVENT' structure is 
 *  destroyed.
 *
 *  @PoolType: Type of memory pool used for allocation.
 *
 *  @Data: Pointer to the 'FLT_CALLBACK_DATA' structure containing callback 
 *  data related to the file system operation.
 *
 *  @FltObjects: Contains pointers to the various objects that are pertinent
 *  to this event.
 *
 *  Return:
 *    - A pointer to the allocated and initialized 'FS_EVENT' structure if 
 *      successful.
 *    - 'NULL' if the allocation or initialization fails.
 */
extern PFS_EVENT
FSEventCreate(
        _PoolType_ POOL_TYPE             PoolType,
        _In_       PFLT_CALLBACK_DATA    Data,
        _In_       PCFLT_RELATED_OBJECTS FltObjects
    );

/*
 *  FSEventCopy() -
 *
 *  Copies the contents of a source 'FS_EVENT' structure to a destination 
 *  'FS_EVENT' structure. This includes copying the process information, 
 *  time details, major function code, and file event details.
 *
 *  @Dest: Pointer to the destination 'FS_EVENT' structure where the 
 *  contents of the source 'FS_EVENT' structure will be copied.
 *
 *  @Src: Pointer to the source 'FS_EVENT' structure from which the contents 
 *  will be copied.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if all elements are successfully copied.
 *    - 'STATUS_UNSUCCESSFUL' if any copy operation fails.
 */
extern NTSTATUS
FSEventCopy(
        _Out_ PFS_EVENT Dest,
        _In_  PFS_EVENT Src
    );

/*
 *  FSEventDestroy() -
 *
 *  This function frees the resources associated with an 'FS_EVENT' structure, 
 *  including the memory allocated for the structure itself and any related 
 *  components like 'FileEvent'. After deallocation, the pointer to the 
 *  'FS_EVENT' structure is set to 'NULL' to prevent dangling pointers.
 *
 *  @FSEvent: Pointer to a pointer of the 'FS_EVENT' structure that is to be 
 *  destroyed. The function deallocates the structure and its associated 
 *  resources only if the pointer is valid.
 */
extern void
FSEventDestroy(
        _Inout_ PFS_EVENT* FSEvent
    );

#endif  /* FS_EVENT_H */
