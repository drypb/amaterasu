
#include "procfltr.h"

/*
 *  ProcFltrCallback() -
 *
 *  Handles process creation and termination notifications. This callback function
 *  is invoked when a process is created or terminated. It updates the tracking
 *  status of the process and records the event.
 *
 *  @PPID: The process ID of the parent process, used to determine the
 *  relationship between processes.
 *
 *  @PID: The process ID of the process being created or terminated.
 *  @Created: A boolean value indicating whether the process is being created
 *  (TRUE) or terminated (FALSE).
 *
 *  This function performs the following actions:
 *    - Looks up the process object by PID.
 *    - Updates the process tracking status based on the creation or termination event.
 *    - Creates and inserts a process event into the list of tracked events.
 */
static void ProcFltrCallback(_In_ HANDLE PPID, _In_ HANDLE PID, _In_ BOOLEAN Created) {

    NTSTATUS  Status;
    PEPROCESS eProc;
    PPROC_EVENT ProcEvent;
    PPROC Proc;

    Status = PsLookupProcessByProcessId(PID, &eProc);
    if(!NT_SUCCESS(Status)) {
        return;
    }

    Proc = AmaterasuLookupNoRef(eProc);
    if(Proc) {
        Status = STATUS_SUCCESS;
        if(!Created) {
            AmaterasuUntrack(Proc);
        }
    } else {
        if(AmaterasuLookupByIdNoRef(PPID)) {
            Proc = ProcCreate(Amaterasu.ProcFltr->PoolType, eProc);
            if(Proc) {
                AmaterasuTrack(Proc);
                Status = STATUS_SUCCESS;
            }
        }
    }

    ObDereferenceObject(eProc);
    if(NT_SUCCESS(Status)) {
        ProcEvent = ProcEventCreate(Amaterasu.ProcFltr->PoolType, PID, Created);
        if(ProcEvent) {
            ListInsert(Amaterasu.ProcFltr->List, ProcEvent);
        }
    }
}

/*
 *  ProcFltrAlloc() - 
 *
 *  Allocates and initializes a file event structure.
 *
 *  @PoolType: The type of memory pool (e.g., paged or nonpaged) 
 *  to allocate the file event from.
 *
 *  Return:
 *    - Pointer to the allocated and initialized 'PROCFLTR' structure.
 *    - 'NULL' if the allocation fails.
 */
static PPROCFLTR ProcFltrAlloc(_PoolType_ POOL_TYPE PoolType) {

    PPROCFLTR ProcFltr;

    ProcFltr = ExAllocatePool2(PoolType, sizeof *ProcFltr 'pftr');
    if(!ProcFltr) {
        return NULL;
    }

    RtlZeroMemory(ProcFltr, sizeof *ProcFltr);

    /*
     *  Store the pool type used for the allocation in 'ProcFltr' to ensure 
     *  correct memory handling, regardless of whether the caller routine is 
     *  paged or nonpaged.
     */
    ProcFltr->PoolType = PoolType;

    return ProcFltr;
}
/*
 *  ProcFltrInit() -
 *  
 *  Initializes the process filter state and registers a callback for process
 *  creation notifications. This function sets up the filter's event copy mechanism,
 *  creates an internal list for managing process events, and registers the
 *  callback using the 'PsSetCreateProcessNotifyRoutine' function.
 *  
 *  @ProcFltr: A pointer to the process filter state structure that needs to
 *  be initialized.
 *
 *  @DriverObj: A pointer to the driver's 'DRIVER_OBJECT' structure, used
 *  during filter initialization.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the filter was successfully initialized, the list
 *      was created, and the process creation notification callback was registered.
 *    - 'STATUS_UNSUCCESSFUL' if the initialization fails, such as when the list
 *      creation or callback registration encounters an error.
 */
static NTSTATUS ProcFltrInit(_Inout_ PPROCFLTR ProcFltr, _Inout_ PDRIVER_OBJECT DriverObj) {

    Assert(ProcFltr);
    Assert(DriverObj);

    ProcFltr->Copy = ProcEventCopy;
    ProcFltr->List = ListCreate(ProcFltr->PoolType, PROCFLTR_MAX, ProcEventDestroy);
    if(ProcFltr->List) {
        return PsSetCreateProcessNotifyRoutine(ProcFltrCallback, FALSE);
    }

    return STATUS_UNSUCCESSFUL;
}

/*
 *  ProcFltrLoad() -
 *
 *  Loads and initializes a process filter associated with the driver. This
 *  function allocates memory for the process filter, initializes it, and
 *  returns a pointer to the filter structure.
 *
 *  @DriverObj: A pointer to the driver's 'DRIVER_OBJECT' structure, which
 *  represents the driver in the system. This object is used during the
 *  initialization of the process filter.
 *
 *  Return:
 *    - A pointer to the initialized 'PROCFLTR' structure if successful.
 *    - 'NULL' if the filter could not be allocated or initialized.
 */
PPROCFLTR ProcFltrLoad(_Inout_ PDRIVER_OBJECT DriverObj) {

    NTSTATUS Status;
    PPROCFLTR ProcFltr;

    Assert(DriverObj);

    ProcFltr = ProcFltrAlloc(POOL_FLAG_NON_PAGED);
    if(ProcFltr) {
        Status = ProcFltrInit(ProcFltr, DriverObj);
        if(!NT_SUCCESS(Status)) {
            ProcFltrUnload(&ProcFltr);
        }
    }

    return ProcFltr;
}
