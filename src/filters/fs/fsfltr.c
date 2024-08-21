
#include "fsfltr.h"

/*
 *  FSFltrDefaultCallback() -
 *
 *  This callback function is invoked for each I/O operation intercepted by the
 *  filter. It checks if the process making the request is of interest by
 *  looking it up with the 'AmaterasuLookup' function. If the process is found,
 *  it creates an 'FSEvent' structure to represent the file system event and
 *  inserts it into the filter's list for further processing.
 *
 *  @Data: A pointer to the 'FLT_CALLBACK_DATA' structure that contains
 *  information about the I/O operation.
 *
 *  @FltObjs: A pointer to the 'FLT_RELATED_OBJECTS' structure that provides
 *  additional context for the operation, such as the file object and volume
 *  object.
 *
 *  @CompletionContext: A pointer to the completion context, which is not used
 *  in this implementation. 
 *
 *  Return:
 *    - 'FLT_PREOP_SUCCESS_NO_CALLBACK': Indicates that the operation should
 *      continue without invoking additional callbacks for this operation.
 */
static FLT_PREOP_CALLBACK_STATUS FSFltrDefaultCallback(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjs, _In_ PVOID* CompletionContext) {

    PEPROCESS eProc;
    PFS_EVENT FSEvent;

    UNREFERENCED_PARAMETER(CompletionContext);

    eProc = FltGetRequestorProcess(Data);
    if(eProc) {
        if(AmaterasuLookupNoRef(eProc)) {
            FSEvent = FSEventCreate(Amaterasu.FSFltr->PoolType, Data, FltObjs);
            if(FSEvent) {
                ListInsert(Amaterasu.FSFltr->List, FSEvent);
            }
        }
        ObDereferenceObject(eProc);
    }

    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}


static const FLT_OPERATION_REGISTRATION Callbacks[] = {
    {
        IRP_MJ_CREATE,
        0,
        FSFltrDefaultCallback,   
        NULL
    },
    {
        IRP_MJ_READ,
        0,
        FSFltrDefaultCallback,   
        NULL
    },
    {
        IRP_MJ_WRITE,
        0,
        FSFltrDefaultCallback,   
        NULL
    },
    {IRP_MJ_OPERATION_END}
};

/*
 *  'FLT_REGISTRATION' structure provides a framework for defining the behavior
 *  of a file system filter driver within the Windows Filter Manager.
 */
static const FLT_REGISTRATION FilterRegistration = {

    sizeof(FLT_REGISTRATION),
    FLT_REGISTRATION_VERSION,

    /*
     *  'FLTFL_REGISTRATION_SUPPORT_NPFS_MSFS' specifies support for Named
     *  Pipes File System (NPFS) and Mailslot File System (MSFS), and
     *  'FLTFL_REGISTRATION_SUPPORT_DAX_VOLUME' specifies support for Direct
     *  Access (DAX) volumes.
     */
    FLTFL_REGISTRATION_SUPPORT_NPFS_MSFS | FLTFL_REGISTRATION_SUPPORT_DAX_VOLUME,
    NULL,
    Callbacks,
    AmaterasuUnload,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};


/*
 *  FSFltrAlloc() - 
 *
 *  Allocates and initializes a file event structure.
 *
 *  @PoolType: The type of memory pool (e.g., paged or nonpaged) 
 *  to allocate the file event from.
 *
 *  Return:
 *    - Pointer to the allocated and initialized 'FSFLTR' structure.
 *    - 'NULL' if the allocation fails. 
 */
static PFSFLTR FSFltrAlloc(_PoolType POOL_TYPE PoolType) {

    PFSFLTR FSFltr;

    FSFltr = ExAllocatePool2(PoolType, sizeof *FSFltr, 'fstr');
    if(FSFltr) {
        RtlZeroMemory(FileEvent, sizeof *FSFltr);
    }

    RtlZeroMemory(FSFltr, sizeof *FSFltr);

    /*
     *  Store the pool type used for the allocation in 'FSFLTR' to ensure 
     *  correct memory handling, regardless of whether the caller routine is 
     *  paged or nonpaged.
     */
    FSFltr->PoolType = PoolType;

    return FSFltr;
}

/*
 *  FSFltrInit() -
 *  
 *  Initializes the filter state and registers the filter with the Filter
 *  Manager. This function sets up the filter's internal list and registers the
 *  filter using the 'FltRegisterFilter' function.
 *  
 *  @FSFltr: A pointer to the filter state structure that needs to
 *  be initialized.
 *
 *  @DriverObj: A pointer to the driver's 'DRIVER_OBJECT' structure, used
 *  during filter registration.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the filter was successfully initialized,
 *      registered, and started.
 *    - An appropriate 'NTSTATUS' error code if any step of the initialization
 *      fails.
 */
static NTSTATUS FSFltrInit(_Inout_ PFSFLTR FSFltr, _Inout_ PDRIVER_OBJECT DriverObj) {

    NTSTATUS Status;

    Assert(FSFltr);
    Assert(DriverObj);


    FSFltr->Copy = FSEventCopy;
    FSFltr->List = ListCreate(FSFltr->PoolType, FSFLTR_MAX, FSEventDestroy);
    if(FSFltr->List) {
        Status = FltRegisterFilter(DriverObject, &FilterRegistration, &FSFltr->FilterHandle);
        if(NT_SUCCESS(Status)) {
            /*
             *  'FltStartFiltering()' notifies the Filter Manager that 'Amaterasu'
             *  is ready to begin attaching to volumes and filtering I/O requests.
             */
            return FltStartFiltering(FSFltr->FilterHandle);
        }
    }

    return STATUS_UNSUCCESSFUL;
}

/*
 *  FSFltrLoad() -
 *
 *  Loads and initializes a file system filter associated with the driver. This
 *  function allocates memory for the filter, initializes it, and returns a
 *  pointer to the filter structure.
 *
 *  @DriverObj: A pointer to the driver's 'DRIVER_OBJECT' structure, which
 *  represents the driver in the system. This object is used during the
 *  initialization of the file system filter.
 *
 *  Return:
 *    - A pointer to the initialized `FSFLTR` structure if successful.
 *    - 'NULL' if the filter could not be loaded or initialized.
 */
PFSFLTR FSFltrLoad(_Inout_ PDRIVER_OBJECT DriverObj) {

    NTSTATUS Status;
    PFSFLTR FSFltr;

    Assert(DriverObj);

    FSFltr = FSFltrAlloc(POOL_FLAG_NON_PAGED);
    if(FSFltr) {
        Status = FSFltrInit(FSFltr, DriverObj);
        if(!NT_SUCCESS(Status)) {
            FSFltrUnload(&FSFltr);
        }
    }

    return FSFltr;
}

/*
 *  FSFltrUnload() -
 *
 *  Unloads the file system filter by unregistering it and releasing any
 *  associated resources. This function ensures that the filter is properly
 *  unregistered and frees the memory allocated for the filter structure.
 *
 *  @FSFltr: A double pointer to the `FSFLTR` structure representing the file
 *  system filter. If the filter is valid, it is unregistered and the memory
 *  associated with it is freed.
 */
void FSFltrUnload(_Inout_ PFSFLTR* FSFltr) {

    if(FSFltr && *FSFltr) {
        FltUnregisterFilter((*FSFltr)->FilterHandle);
        ListDestroy(&(*FSFltr)->List);
        ExFreePoolWithTag((*FsFltr));
        *FsFltr = NULL;
    }
}
