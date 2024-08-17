#ifndef FILE_EVENT_H
#define FILE_EVENT_H

#include "comm.h"

struct _FILE_EVENT {

    POOL_TYPE PoolType;

    WCHAR Name[MAX_PATH];
    WCHAR Path[MAX_PATH];
};

typedef struct _FILE_EVENT FILE_EVENT, *PFILE_EVENT;

/*
 *  FileEventCreate() - 
 *
 *  Allocates and initializes a FILE_EVENT structure based on the 
 *  provided pool type and callback data.
 *
 *  @PoolType: The memory pool type to use for allocating 
 *  the 'FILE_EVENT' structure.
 *
 *  @Data: Pointer to the callback data containing information 
 *  about the I/O operation.
 *
 *  Return:
 *    - Pointer to the initialized 'FILE_EVENT' structure if successful.
 *    - 'NULL' if allocation or initialization fails.
 */
extern PFILE_EVENT*
FileEventCreate(
        _PoolType_ POOL_TYPE            PoolType,
        _In_       PFLT_CALLBACK_DATA   Data
    );

/*
 *  FileEventCopy() - 
 *
 *  Copies the file name and path information from a source 'FileEvent'
 *  structure to a destination 'FileEvent' structure.
 *
 *  @Dest: Pointer to the destination user mode 'PFILE_EVENT' structure 
 *  where the file name and path information will be copied.
 *
 *  @Src: Pointer to the source 'PFILE_EVENT' structure from which the 
 *  file name and path information will be copied.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the copy operation is successful.
 *    - 'STATUS_UNSUCCESSFUL' if the copy operation fails, typically 
 *      due to invalid memory access.
 */
extern NTSTATUS 
FileEventCopy(
        _Out_ PFILE_EVENT Dest,
        _In_ PFILE_EVENT  Src
    );

/*
 *  FileEventDestroy() - 
 *
 *  Deallocates memory used by a 'PFILE_EVENT' structure and sets 
 *  the pointer to 'NULL'.
 *
 *  @FileEvent: Pointer to a 'PFILE_EVENT' pointer that points to the 
 *  file event structure to be destroyed. On successful destruction, the 
 *  pointer is set to 'NULL'.
 */
extern void
FileEventDestroy(
        _Inout_ PFILE_EVENT* FileEvent
    );

#endif  /* FILE_EVENT_H */
