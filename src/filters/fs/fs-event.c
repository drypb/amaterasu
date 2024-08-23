
#include "proc.h"
#include "time.h"

#include "file-event.h"

/*
 *  FSEventAlloc() - 
 *
 *  Allocates and initializes a file event structure.
 *
 *  @PoolType: The type of memory pool (e.g., paged or nonpaged) 
 *  to allocate the file event from.
 *
 *  Return:
 *    - Pointer to the allocated and initialized 'FS_EVENT' structure.
 *    - 'NULL' if the allocation fails.
 */
static PFS_EVENT FSEventAlloc(_PoolType_ POOL_TYPE PoolType) {

    PFS_EVENT FSEvent;

    FSEvent = ExAllocatePool2(PoolType, sizeof *FSEvent 'fs');
    if(!FSEvent) {
        return NULL;
    }

    RtlZeroMemory(FSEvent, sizeof *FSEvent);

    /*
     *  Store the pool type used for the allocation in 'FSEvent' to ensure 
     *  correct memory handling, regardless of whether the caller routine is 
     *  paged or nonpaged.
     */
    FSEvent->PoolType = PoolType;

    return FSEvent;
}

/*
 *  FSEventInitProc() -
 *
 *  Initializes the process-related information in the 'FS_EVENT' structure 
 *  by retrieving or creating a 'PROC' structure associated with the process 
 *  making the file system request. The function also ensures that the process 
 *  is being tracked by the system and updates its token information if needed.
 *
 *  @FSEvent: Pointer to the 'FS_EVENT' structure where the process information 
 *  will be initialized.
 *
 *  @Data: Pointer to the 'FLT_CALLBACK_DATA' structure containing callback 
 *  data for the file system operation.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the process information is successfully initialized 
 *      and the 'PROC' structure is either retrieved or created and inserted 
 *      into the tracking system.
 *    - 'STATUS_UNSUCCESSFUL' if the process cannot be retrieved.
 */
static NTSTATUS FSEventInitProc(_Inout_ PFS_EVENT FSEvent, _In_ PFLT_CALLBACK_DATA Data) {

    NTSTATUS Status;
    PEPROCESS eProc;

    Assert(FSEvent);
    Assert(Data);

    Status = STATUS_UNSUCCESSFUL;
    eProc  = FltGetRequestorProcess(Data);
    if(!eProc) {
        return Status;
    }

    /*
     *  This code retrieves the existing 'PROC' structure associated with the
     *  requestor process from the AVL tree that maintains the list of
     *  monitored processes. Given that the process is already being tracked,
     *  this lookup should never fail.
     */
    FSEvent->Proc = AmaterasuLookup(eProc);
    if(FSEvent->Proc) {
        TokenUpdate(FSEvent->Proc->Token, eProc);
        Status = STATUS_SUCCESS;
    }

    ObDereferenceObject(eProc);

    return Status;
}

/*
 *  FSEventInitOptions() - 
 *
 *  Initializes specific options for a file system event based on the IRP major
 *  function code. This function extracts relevant options from the provided
 *  callback data (IRP) and stores them in the 'FSEvent' structure.
 *
 *  @FSEvent: A pointer to the 'FS_EVENT' structure that will be updated with
 *  the appropriate options based on the IRP major function.
 *
 *  @Data: A pointer to the 'FLT_CALLBACK_DATA' structure containing the IRP
 *  information used to determine and set the options in 'FSEvent'.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the initialization is successful.
 *    - Appropriate 'NTSTATUS' error code if an error occurs during
 *      initialization.
 */
static NTSTATUS FSEventInitOptions(_Inout_ PFS_EVENT FSEvent, _In_ PFLT_CALLBACK_DATA Data) {

    PIO_STACK_LOCATION IrpIoStack;

    Assert(FSEvent);
    Assert(Data);

    IrpIoStack = IoGetCurrentIrpStackLocation(Data->Irp);
    if(!IrpIoStack) {
        return STATUS_UNSUCCESSFUL;
    }

    switch(FSEvent->MjFunc) {
        /*
         * Handles IRP_MJ_CREATE, which is triggered when a file or device is
         * opened. This is useful for detecting malware's file or device access 
         * patterns, such as creation or opening of files which may be part of 
         * malicious activity or file manipulation.
         */
        case IRP_MJ_CREATE:
            FSEvent->Options = IrpIoStack->Parameters.Create.Options;
            break;

        /*
         * Handles IRP_MJ_SET_INFORMATION, which is used to modify file
         * information. This information helps in understanding the changes
         * malware makes to file attributes or properties, such as altering
         * file permissions or timestamps to evade detection.
         */
        case IRP_MJ_SET_INFORMATION:
            FSEvent->Options = IrpIoStack->Parameters.SetFile.FileInformationClass;
            break;

        /*
         * Handles IRP_MJ_QUERY_INFORMATION, which queries file or device
         * information. This is valuable for identifying what details malware 
         * seeks about files or devices, potentially revealing its intent to 
         * access or manipulate specific system resources.
         */
        case IRP_MJ_QUERY_INFORMATION:
            FSEvent->Options = IrpIoStack->Parameters.QueryFile.FileInformationClass;
            break;

        /*
         * Handles IRP_MJ_DEVICE_CONTROL, which manages device-specific control
         * operations. This is useful for monitoring custom commands sent by
         * malware to device drivers, which might be used for malicious control
         * or data manipulation.
         */
        case IRP_MJ_DEVICE_CONTROL:
            FSEvent->Options = IrpIoStack->Parameters.DeviceIoControl.FileInformationClass;
            break;

        /*
         * Handles IRP_MJ_DIRECTORY_CONTROL, which queries or manipulates
         * directory contents. This helps in analyzing how malware interacts with
         * directory structures, such as listing or altering directory contents 
         * which may be indicative of attempts to hide or manipulate files.
         */
        case IRP_MJ_DIRECTORY_CONTROL:
            FSEvent->Options = IrpIoStack->Parameters.QueryDirectory.FileInformationClass;
    }

    return STATUS_SUCCESS;
}

/*
 *  FSEventInit() -
 *
 *  Initializes a 'FS_EVENT' structure by setting up the time of the event, 
 *  and optionally creating a 'FILE_EVENT' structure if a 'FileObject' is 
 *  present in the filter-related objects. Initializes process information 
 *  related to the event.
 *
 *  @FSEvent: Pointer to the 'FS_EVENT' structure to be initialized.
 *
 *  @Data: Pointer to the 'FLT_CALLBACK_DATA' structure containing callback 
 *  data for the file system operation.
 *
 *  @FltObjects: Pointer to the 'CFLT_RELATED_OBJECTS' structure containing 
 *  filter-related objects.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the 'FS_EVENT' structure is successfully 
 *      initialized.
 *    - 'STATUS_UNSUCCESSFUL' if the initialization fails, typically due to 
 *      failure in creating the 'FILE_EVENT' structure.
 */
static NTSTATUS FSEventInit(_Inout_ PFS_EVENT FSEvent, _In_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects) {

    Assert(FSEvent);
    Assert(Data);
    Assert(FltObjects);

    TimeInit(&FSEvent->Time);
    FSEvent->MjFunc = MAJOR_FUNCTION(Data);

    /*
     *  It is not possible to get a name when
     *  there's no 'FileObject'.
     *
     *  In this case, we report everything except the
     *  the contents of 'FILE_EVENT'.
     */
    if(FltObjects->FileObject) {
        FSEvent->FileEvent = FileEventCreate(FSEvent->PoolType, Data);
        if(!FSEvent->FileEvent) {
            return STATUS_UNSUCCESSFUL;
        }
    }

    Status = FSEventInitProc(FSEvent, Data);
    if(NT_SUCCESS(Status)) {
        Status = FSEventInitOptions(FSEvent, Data);
    }

    return Status;
}

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
PFS_EVENT FSEventCreate(_PoolType_ POOL_TYPE PoolType, _In_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects) {
    
    NSTATUS Status;
    PFS_EVENT FSEvent;

    Assert(Data);

    FSEvent = FSEventAlloc(PoolType);
    if(FSEvent) {
        Status = FSEventInit(FSEvent, Data, FltObjects);
        if(!NT_SUCCESS(Status)) {
            FSEventDestroy(&FSEvent);
        }
    }

    return FSEvent;
}

/*
 *  FSEventCopyMeta() -
 *
 *  @Dest:
 *  @Src:
 *
 *  Return:
 *    -
 *    -
 */
static NTSTATUS FSEventCopyMeta(_Out_ PFS_EVENT Dest, _In_ PFS_EVENT Src) {

    NTSTATUS Status;

    IF_SUCCESS(Status,
        ProcCopy(Dest->Proc , Src->Proc),
        TimeCopy(&Dest->Time, &Src->Time),
        FileEventCopy(Dest->FileEvent, Src->FileEvent),
    );

    return Status;
}

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
NTSTATUS FSEventCopy(_Out_ PFS_EVENT Dest, _In_ PFS_EVENT Src) {

    NTSTATUS Status;

    Assert(Dest);
    Assert(Src);

    Status = STATUS_UNSUCCESSFUL;
    if(Dest && Src) {
        IF_SUCCESS(Status,
            FSEventCopyMeta(Dest, Src),
            CopyToUserMode(&Dest->MjFunc , &Src->MjFunc  , sizeof Src->MjFunc),
            CopyToUserMode(&Dest->Options, &Dest->Options, sizeof Src->Options)
        );
    }

    return Status;
}

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
void FSEventDestroy(_Inout_ PFS_EVENT* FSEvent) {

    if(FSEvent && *FSEvent) {
        ProcUnref((*FSEvent)->Proc);
        /*
         *  The 'Proc' field in the 'FSEvent' structure is not destroyed here 
         *  because it is merely a reference to an object in the AVL tree that 
         *  tracks processes monitored by Amaterasu.
         */
        FileEventDestroy(&(*FSEvent)->FileEvent);
        ExFreePoolWithTag((*FSEvent));
        *FSEvent = NULL;
    }
}
