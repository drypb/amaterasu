#ifndef UTILS_H
#define UTILS_H

#include "utils.defs.h"

/*
 *  CopyToUserMode() -
 *
 *  Copies data from a source buffer to a destination buffer in user mode. This
 *  function uses 'MmCopyMemory' to safely transfer data, handling any potential
 *  exceptions that may occur during the operation.
 *
 *  @Dest: A pointer to the destination buffer in user mode where the data will
 *  be copied.
 *
 *  @Src: A pointer to the source buffer containing the data to be copied.
 *  @Size: The size, in bytes, of the data to be copied from the source to the
 *  destination.
 *
 *  @Bytes: A pointer to a variable that will receive the number of bytes
 *  successfully transferred. This parameter is optional and can be NULL.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the data was successfully copied.
 *    - 'STATUS_UNSUCCESSFUL' if an exception occurred during the copy or if
 *      the parameters were invalid.
 */
extern NTSTATUS 
CopyToUserMode(
        _Out_ PVOID   Dest,
        _In_  PVOID   Src,
        _In_  SIZE_T  Size,
        _Out_ PSIZE_T Bytes
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
        _PoolType_ POOL_TYPE       PoolType, 
        _Inout_    PUNICODE_STRING UniStr, 
        _In_       SIZE_T          Size
    );

#endif  /* UTILS_H */
