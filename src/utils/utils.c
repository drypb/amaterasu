
#include "utils.h"

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
NTSTATUS CopyToUserMode(_Out_ PVOID Dest, _In_ PVOID Src, _In_ SIZE_T Size) {
    
    if(Dest && Src) {
        __try {
            PROBE_AND_COPY(Dest, Src, Size);
        } __except() {
            return STATUS_UNSUCCESSFUL;
        }
    }

    return STATUS_SUCCESS;
}
