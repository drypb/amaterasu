#ifndef UTILS_H
#define UTILS_H

#include "utils.defs.h"

/*
 *  CopyToUserMode() -
 *
 *  Copies a block of data from kernel mode to user mode, ensuring that
 *  the copy operation is safely performed and any exceptions are handled.
 *
 *  @Dest: Pointer to the destination buffer in user mode where the data 
 *  will be copied to.
 *
 *  @Src: Pointer to the source buffer in kernel mode from which the data 
 *  will be copied.
 *
 *  @Size: The size of the data to be copied, in bytes.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the data is successfully copied to user mode.
 *    - 'STATUS_UNSUCCESSFUL' if an exception occurs during the copy operation.
 *
 */
extern NTSTATUS 
CopyToUserMode(
        _Out_ PVOID  Dest,
        _In_  PVOID  Src,
        _In_  SIZE_T Size
    );

/*
 *  InitUnicodeString() -
 *
 *  Allocates and initializes a Unicode string structure with a specified 
 *  size. The buffer for the Unicode string is allocated using the given 
 *  pool type and size.
 *
 *  @PoolType: Specifies the pool type for the buffer allocation.
 *
 *  @UniStr: Pointer to the 'UNICODE_STRING' structure to be initialized.
 *
 *  @Size: Size of the buffer to allocate for the Unicode string.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the buffer is successfully allocated and the 
 *      Unicode string is initialized.
 *    - 'STATUS_UNSUCCESSFUL' if the buffer allocation fails.
 *
 *  Note:
 *    - The caller is responsible for freeing the allocated buffer using 
 *      'ExFreePoolWithTag' when it is no longer needed.
 */
extern NTSTATUS 
InitUnicodeString(
        _PoolType_ POOL_TYPE PoolType, 
        _Inout_ PUNICODE_STRING UniStr, 
        _In_ SIZE_T Size
    );

#endif  /* UTILS_H */
