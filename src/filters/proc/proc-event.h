#ifndef PROC_EVENT_H
#define PROC_EVENT_H

struct _PROC_EVENT {
 
    POOL_TYPE PoolType;
    PPROC     Proc;
    BOOLEAN   Created;
};

typedef struct _PROC_EVENT PROC_EVENT, *PPROC_EVENT;

/*
 *  ProcEventCreate() - 
 *
 *  Allocates and initializes a process event structure.
 *
 *  @PoolType: Memory pool from which the process event structure is allocated.
 *  @PID: Handle to the process ID for which the event is being created.
 *  @Created: Boolean indicating whether the process was newly created (TRUE)
 *             or already existing (FALSE).
 *
 *  Return:
 *    - A pointer to the initialized process event structure if successful.
 *    - 'NULL' if the process event structure could not be created or
 *      initialized.
 */
extern PPROC_EVENT
ProcEventCreate(
        _PoolType_ POOL_TYPE PoolType,
        _In_       HANDLE    PPID,
        _In_       HANDLE    PID
    );

/*
 *  ProcEventCopy() - 
 *
 *  Copies the process event information, including the process structure
 *  and creation status, from a source 'ProcEvent' structure to a 
 *  destination 'ProcEvent' structure.
 *
 *  @Dest: Pointer to the destination 'PPROC_EVENT' structure where the 
 *  process information will be copied.
 *
 *  @Src: Pointer to the source 'PPROC_EVENT' structure from which the 
 *  process information will be copied.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the copy operation is successful.
 *    - 'STATUS_UNSUCCESSFUL' if the copy operation fails, typically due 
 *      to invalid memory access or unsuccessful process copying.
 */
extern NTSTATUS
ProcEventCopy(
        _Out_ PPROC_EVENT Dest,
        _In_  PPROC_EVENT Src
    );

/*
 *  ProcEventDestroy() - 
 *
 *  Frees memory allocated for a 'PROC_EVENT' structure.
 *
 *  @ProcEvent: Pointer to the 'PROC_EVENT' structure to be destroyed. 
 *  After the function executes, the pointer is set to 'NULL'.
 */
extern void
ProcEventDestroy(
        _Inout_ PPROC_EVENT* ProcEvent
    );

#endif  /* PROC_EVENT_H */
