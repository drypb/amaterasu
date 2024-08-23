
#include "proc-event.h"

/*
 *  ProcEventAlloc() - 
 *
 *  Allocates and initializes a file event structure.
 *
 *  @PoolType: The type of memory pool (e.g., paged or nonpaged) 
 *  to allocate the file event from.
 *
 *  Return:
 *    - Pointer to the allocated and initialized 'PROC_EVENT' structure.
 *    - 'NULL' if the allocation fails.
 */
static PPROC_EVENT ProcEventAlloc(_PoolType_ POOL_TYPE PoolType) {

    PPROC_EVENT ProcEvent;

    ProcEvent = ExAllocatePool2(PoolType, sizeof *ProcEvent 'pevt');
    if(!ProcEvent) {
        return NULL;
    }

    RtlZeroMemory(ProcEvent, sizeof *ProcEvent);

    /*
     *  Store the pool type used for the allocation in 'ProcEvent' to ensure 
     *  correct memory handling, regardless of whether the caller routine is 
     *  paged or nonpaged.
     */
    ProcEvent->PoolType = PoolType;

    return ProcEvent;
}

/*
 *  ProcEventInit() - 
 *
 *  Initializes a process event structure by looking up the process associated
 *  with a given PID and setting the event creation status.
 *
 *  @ProcEvent: Pointer to the process event structure to be initialized.
 *
 *  @PID: Handle to the process ID for which the event is being initialized.
 *
 *  @Created: Boolean indicating whether the process was newly created (TRUE)
 *  or already existing (FALSE).
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the process event was successfully initialized.
 *    - 'STATUS_UNSUCCESSFUL' if the process event could not be initialized, 
 *      typically due to failure in looking up the process or finding it in the 
 *      Amaterasu lookup table.
 */
static NTSTATUS ProcEventInit(_Inout_ PPROC_EVENT ProcEvent, _In_ HANDLE PID, _In_ BOOLEAN Created) {

    NTSTATUS Status;
    PEPROCESS eProc;

    Assert(ProcEvent);

    Status = PsLookupProcessByProcessId(PID, &eProc); 
    if(NT_SUCCESS(Status)) {

        ProcEvent->Created = Created;
        ProcEvent->Proc    = AmaterasuLookup(eProc);
        if(ProcEvent->Proc) {
            Status = STATUS_SUCCESS;
            if(!Created) {
                /*
                 *  As this event is triggered only when a process is created
                 *  or destroyed, the token update is necessary only when the
                 *  process is being destroyed. This ensures the token reflects
                 *  the most recent state before the process termination is
                 *  reported.
                 */
                TokenUpdate(ProcEvent->Proc->Token, eProc);
            }
        } 
        ObDereferenceObject(eProc);
    }

    return Status;
}

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
PPROC_EVENT ProcEventCreate(_PoolType_ POOL_TYPE PoolType, _In_ HANDLE PID, _In_ BOOLEAN Created) {

    NTSTATUS Status;
    PPROC_EVENT ProcEvent;

    ProcEvent = ProcEventAlloc(PoolType);
    if(ProcEvent) {
        Status = ProcEventInit(ProcEvent, PID, Created);
        if(!NT_SUCCESS(Status)) {
            ProcEventDestroy(&ProcEvent);
        }
    }

    return Status;
}

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
NTSTATUS ProcEventCopy(_Out_ PPROC_EVENT Dest, _In_ PPROC_EVENT Src) {

    NTSTATUS Status;

    Status = STATUS_UNSUCCESSFUL;
    if(Dest && Src) {
        IF_SUCCESS(Status,
            ProcCopy(Dest->Proc, Dest->Src),
            CopyToUserMode(&Dest->Created, &Src->Created, sizeof Src->Created);
        );
    }

    return Status;
}

/*
 *  ProcEventDestroy() - 
 *
 *  Frees memory allocated for a 'PROC_EVENT' structure.
 *
 *  @ProcEvent: Pointer to the 'PROC_EVENT' structure to be destroyed. 
 *  After the function executes, the pointer is set to 'NULL'.
 */
void ProcEventDestroy(_Inout_ PPROC_EVENT* ProcEvent) {

    if(ProcEvent && *ProcEvent) {
        ProcUnref((*ProcEvent)->Proc);
        /*
         *  The 'Proc' field in the 'PROC_EVENT' structure is not destroyed here 
         *  because it is merely a reference to an object in the AVL tree that 
         *  tracks processes monitored by Amaterasu.
         */
        ExFreePoolWithTag(*ProcEvent, 'pevt');
        *ProcEvent = NULL;
    }
}
