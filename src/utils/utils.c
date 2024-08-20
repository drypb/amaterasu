
#include "utils.h"

/*
 *  CopyToUserMode() -
 *
 *  Copies data from a kernel mode buffer to a user mode buffer, ensuring
 *  proper alignment and exception handling to safely manage potential errors
 *  during the copy operation. If an exception is caught, the function returns
 *  'STATUS_UNSUCCESSFUL'. Otherwise, it returns 'STATUS_SUCCESS' after a
 *  successful copy.
 *
 *  @Dest: A pointer to the destination buffer in user mode where the data will
 *  be copied.
 *
 *  @Src: A pointer to the source buffer in kernel mode from which the data
 *  will be copied.
 *
 *  @Size: The size, in bytes, of the data to be copied.
 *  @Align: The alignment requirement for the data to be copied.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the data was successfully copied.
 *    - 'STATUS_UNSUCCESSFUL' if an exception occurred during the copy.
 */
NTSTATUS (CopyToUserMode)(_Out_ PVOID Dest, _In_ PVOID Src, _In_ SIZE_T Size, _In_ SIZE_T Align) {
    
    if(Dest && Src) {
        __try {
            PROBE_AND_COPY(Dest, Src, Size, Align);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            return STATUS_UNSUCCESSFUL;
        }
    }

    return STATUS_SUCCESS;
}

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
NTSTATUS InitUnicodeString(_PoolType_ POOL_TYPE PoolType, _Inout_ PUNICODE_STRING UniStr, _In_ SIZE_T Size) {

    if(UniStr) {
        UniStr->Buffer = ExAllocatePool2(PoolType, Size, 'ubuf');
        if(UniStr->Buffer) {
            UniStr->Length = Size;
            UniStr->MaximumLength = Size;

            return STATUS_SUCCESS;
        }
    }

    return STATUS_UNSUCCESSFUL;
}
