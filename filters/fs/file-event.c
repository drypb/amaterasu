
#include "file-event.h"
#include "file-event.defs.h"

#include "utils.h"

/*
 *  FileEventAlloc() - 
 *
 *  Allocates and initializes a file event structure.
 *
 *  @PoolType: The type of memory pool (e.g., paged or nonpaged) 
 *  to allocate the file event from.
 *
 *  Return:
 *    - Pointer to the allocated and initialized 'FILE_EVENT' structure.
 *    - 'NULL' if the allocation fails.
 */
static PFILE_EVENT FileEventAlloc(_PoolType_ POOL_TYPE PoolType) {

    PFILE_EVENT FileEvent;

    FileEvent = ExAllocatePool2(PoolType, sizeof *FileEvent 'file');
    if(!FileEvent) {
        return NULL;
    }

    RtlZeroMemory(FileEvent, sizeof *FileEvent);

    /*
     *  Store the pool type used for the allocation in 'FileEvent' to ensure 
     *  correct memory handling, regardless of whether the caller routine is 
     *  paged or nonpaged.
     */
    FileEvent->PoolType = PoolType;

    return FileEvent;
}

/*
 *  AcquireFltNameInfo() - 
 *
 *  Acquires the file name information for a given file operation.
 *
 *  @NameInfo: Pointer to the location where the acquired file name 
 *  information will be stored.
 *
 *  @Data: Pointer to the callback data that contains information 
 *  about the I/O operation.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the initialization is successful.
 *    - Appropriate 'NTSTATUS' error code if an error occurs during
 *      initialization.
 */
static NTSTATUS AcquireFltNameInfo(_Out_ PFLT_FILE_NAME_INFORMATION* NameInfo, _In_ PFLT_CALLBACK_DATA Data) {

    NTSTATUS Status;

    Assert(NameInfo);
    Assert(Data);

    Status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | QUERY_METHOD, NameInfo);
    if(NT_SUCCESS(Status)) {

        /*
         *  If it was not possible to get the "normalized" name, we try to get
         *  the "opened" name. The opened name refers to the name of the file
         *  as it was specified when the file was opened, including the full
         *  path if available.
         */
        Status = FltGetFileNameInformation(Data, FLT_FILE_NAME_OPENED | QueryMethod, NameInfo);
        if(!NT_SUCCESS(Status)) {
            return Status;
        }
    }

    return Status;
}

/*
 *  FileEventInitNames() -
 *
 *  Initializes the path and file name information in the given 
 *  'FileEvent' structure.
 *
 *  @FileEvent: Pointer to the 'PFILE_EVENT' structure that will be 
 *  initialized with the file path and name.
 *
 *  @NameInfo: Pointer to the 'PFLT_FILE_NAME_INFORMATION' structure 
 *  containing the file path and name information.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the initialization is successful.
 *    - Appropriate 'NTSTATUS' error code if an error occurs during
 *      initialization.
 */
static NTSTATUS FileEventInitNames(_Inout_ PFILE_EVENT FileEvent, _In_ PFLT_FILE_NAME_INFORMATION NameInfo) {

    NTSTATUS Status;
    SIZE_T PathLen;
    SIZE_T NameLen;

    Assert(FileEvent);
    Assert(NameInfo);

    Status  = STATUS_UNSUCCESSFUL;

    PathLen = NameInfo->Name.Length;
    NameLen = NameInfo->FinalComponent.Length;

    __try {
        if(PathLen && PathLen < MAX_PATH) {
            RtlCopyMemory(&FileEvent->Path, &NameInfo->Name, NameInfo->Name.Length);
        }
        
        if(NameLen && NameLen < MAX_PATH) {
            RtlCopyMemory(&FileEvent->Name, &NameInfo->FinalComponent, NameLen);
            Status = STATUS_SUCCESS;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        /*
         *  Do nothing, 'Status = STATUS_UNSUCCESSFUL' at the beginning
         *  implicitly handles an exception being throw as it already
         *  indicates an error has occurred.
         */
    }

    return Status;
}

/*
 *  FileEventInit() - 
 *
 *  Initializes a 'FILE_EVENT' structure with process and token information.
 *
 *  @FileEvent: Pointer to the 'FILE_EVENT' structure to initialize.
 *
 *  @Data: Pointer to the callback data that contains information 
 *  about the I/O operation.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the initialization is successful.
 *    - Appropriate 'NTSTATUS' error code if an error occurs during
 *      initialization.
 */
static NTSTATUS FileEventInit(_Inout_ PFILE_EVENT FileEvent, _In_ PFLT_CALLBACK_DATA Data) {

    NTSTATUS Status;
    PFLT_FILE_NAME_INFORMATION NameInfo;

    Assert(FileEvent);
    Assert(Data);

    NameInfo = NULL;
    Status = AcquireFltNameInfo(&NameInfo, Data);
    if(NT_SUCCESS(Status)) {
        Status = FltParseFileNameInformation(NameInfo);
        if(NT_SUCCESS(Status)) {
            Status = FileEventInitNames(FileEvent, NameInfo);
        }
    }

    if(NameInfo) {
        FltReleaseFileNameInformation(NameInfo);
    }

    return Status;
}

/*
 *  FileEventCreate() - 
 *
 *  Allocates and initializes a FILE_EVENT structure based on the 
 *  provided pool type and callback data.
 *
 *  @PoolType: The memory pool type to use for allocating 
 *  the 'FILE_EVENT' structure.
 *
 *  @Data: Pointer to the callback data containing information 
 *  about the I/O operation.
 *
 *  Return:
 *    - Pointer to the initialized 'FILE_EVENT' structure if successful.
 *    - 'NULL' if allocation or initialization fails.
 */
PFILE_EVENT FileEventCreate(_PoolType_ POOL_TYPE PoolType, _In_ PFLT_CALLBACK_DATA Data) {

    NTSTATUS Status;
    PFILE_EVENT FileEvent;

    Assert(PoolType);
    Assert(Data);

    FileEvent = FileEventAlloc(PoolType);
    if(FileEvent) {
        Status = FileEventInit(FileEvent, Data);
        if(!NT_SUCCESS(Status)) {
            FilEventDestroy(&FileEvent);            
        }
    }

    return FileEvent;
}

/*
 *  FileEventCopy() - 
 *
 *  Copies the file name and path information from a source 'FileEvent'
 *  structure to a destination 'FileEvent' structure.
 *
 *  @Dest: Pointer to the destination user mode 'PFILE_EVENT' structure 
 *  where the file name and path information will be copied.
 *
 *  @Src: Pointer to the source 'PFILE_EVENT' structure from which the 
 *  file name and path information will be copied.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the copy operation is successful.
 *    - 'STATUS_UNSUCCESSFUL' if the copy operation fails, typically 
 *      due to invalid memory access.
 */
NTSTATUS FileEventCopy(_Out_ PFILE_EVENT Dest, _In_ PFILE_EVENT Src) {

    NTSTATUS Status;

    if(Dest && Src) {
        IF_SUCCESS(Status,
            CopyToUserMode(Dest->Name, Src->Name, sizeof Src->Name), 
            CopyToUserMode(Dest->Path, Src->Path, sizeof Src->Path)
        );
    }

    return Status;
}

/*
 *  FileEventDestroy() - 
 *
 *  Deallocates memory used by a 'PFILE_EVENT' structure and sets 
 *  the pointer to 'NULL'.
 *
 *  @FileEvent: Pointer to a 'PFILE_EVENT' pointer that points to the 
 *  file event structure to be destroyed. On successful destruction, the 
 *  pointer is set to 'NULL'.
 */
void FileEventDestroy(_Inout_ PFILE_EVENT* FileEvent) {

    if(FileEvent && *FileEvent) {
        ExFreePoolWithTag(*FileEvent, 'file');
        *FileEvent = NULL;
    }
}
