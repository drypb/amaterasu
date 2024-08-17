#ifndef UTILS_DEFS_H
#define UTILS_DEFS_H

/*
 *  PROBE_AND_COPY() -
 *
 *  Safely copies a block of memory from kernel mode to user mode. The macro
 *  ensures that the destination buffer is writable by the user mode process
 *  before performing the memory copy operation.
 *
 *  @Dest: Pointer to the destination buffer in user mode where the data
 *  will be copied to. The buffer must be writable.
 *
 *  @Src: Pointer to the source buffer in kernel mode from which the data
 *  will be copied. The buffer is read-only.
 *
 *  @Size: The size of the data to be copied, in bytes.
 *
 *  Note:
 *    The macro uses `ProbeForWrite` to validate that the user mode buffer
 *    is accessible for writing and `RtlCopyMemory` to perform the actual
 *    copy. The alignment of the source buffer is used for the probe operation.
 */
#define PROBE_AND_COPY(Dest, Src, Size)               \
    do {                                              \
        ProbeForWrite(Dest, Src, __alignof(*Src));    \
        RtlCopyMemory(Dest, Src, Size);               \
    } while(0)

#endif  /* UTILS_DEFS_H */
