
#include "proc.h"
#include "proc.defs.h"

/*
 *  ProcAlloc() - 
 *
 *  Allocate a 'PROC' structure.
 *
 *  @PoolType: The type of memory pool to allocate from 
 *  (paged or nonpaged).
 *
 *  Return:
 *    - Pointer to the allocated 'PROC' structure on success.
 *    - 'NULL' if memory allocation fails.
 */
static PPROC ProcAlloc(_PoolType_ POOL_TYPE PoolType) {

    PPROC Proc;

    Proc = ExAllocatePool2(PoolType, sizeof *Proc, 'proc');
    if(!Proc) {
        return NULL;
    }

    RtlZeroMemory(Proc, sizeof *Proc);

    /*
     *  Store the pool type used for the allocation in 'Proc' to ensure 
     *  correct memory handling, regardless of whether the caller routine is 
     *  paged or nonpaged.
     */
    Proc->PoolType = PoolType;

    return Proc;
}

/*
 *  ProcInitIDs() -
 *
 *  Initializes the process identifiers and session ID in a 'PROC' structure 
 *  using information from a given 'EPROCESS' structure.
 *
 *  @Proc: Pointer to the 'PROC' structure to be initialized with process 
 *  identifiers and session ID.
 *
 *  @eProc: Pointer to an 'EPROCESS' structure representing the process 
 *  from which identifiers and session ID will be retrieved.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the identifiers are successfully initialized.
 *    - This function currently returns 'STATUS_SUCCESS' but future 
 *      implementations may include additional error handling and status codes.
 */
static NTSTATUS ProcInitIDs(_Inout_ PPROC Proc, _In_ PEPROCESS eProc) {

    Assert(Proc);
    Assert(eProc);

    Proc->PPID = eProc->InheritedFromUniqueProcessId;
    Proc->PID  = PsGetProcessId(eProc);
    Proc->SID  = MmGetSessionId(eProc);

    return STATUS_SUCCESS;
}

/*
 *  ProcInitImage() -
 *
 *  Initializes the image name for a given process. This function allocates 
 *  memory for the image name, retrieves it using SeLocateProcessImageName, 
 *  and copies the image name to the provided process structure. It also 
 *  ensures proper cleanup of allocated memory.
 *
 *  @Proc: Pointer to the 'PROC' structure where the image name will be stored.
 *
 *  @eProc: Pointer to the 'EPROCESS' structure representing the process from 
 *          which the image name will be retrieved.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the image name is successfully retrieved and copied.
 *    - 'STATUS_UNSUCCESSFUL' if the image name could not be retrieved or if 
 *      any errors occurred during the process.
 */
static NTSTATUS ProcInitImage(_Inout_ PPROC Proc, _In_ PEPROCESS eProc) {

    UNICODE_STRING  Image;
    PUNICODE_STRING pImage;

    Assert(Proc);
    Assert(eProc);

    pImage = &Image;
    Status = InitUnicodeString(Proc->PoolType, pImage, MAX_STR_SIZE);
    if(NT_SUCCESS(Status)) {
        Status = SeLocateProcessImageName(eProc, &pImage);
        if(NT_SUCCESS(Status)) {
            RtlCopyMemory(Proc->Image, pImage->Buffer, MAX_PATH_WCHAR);
            ExFreePoolWithTag(pImage->Buffer, 'ubuf');
            return STATUS_SUCCESS;
        }
    }

    return STATUS_UNSUCCESSFUL;
}

/*
 *  ProcInit() -
 *
 *  Initializes a 'PROC' structure by setting up process identifiers, image 
 *  information, and creating a token associated with the process.
 *
 *  @Proc: Pointer to the 'PROC' structure to be initialized. This structure
 *  will hold various attributes related to the process, including IDs, 
 *  image information, and a security token.
 *
 *  @eProc: Pointer to an 'EPROCESS' structure representing the process 
 *  for which the 'PROC' structure will be initialized.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the process information and token are successfully
 *      initialized.
 *    - An appropriate NTSTATUS error code if any of the initialization 
 *      steps fail.
 */
static NTSTATUS ProcInit(_Inout_ PPROC Proc, _In_ PEPROCESS eProc) {

    NTSTATUS Status;

    Assert(Proc);
    Assert(eProc);

    Status = ProcInitIDs(Proc, eProc);
    if(NT_SUCCESS(Status)) {
        Status = ProcInitImage(Proc, eProc);
        if(NT_SUCCESS(Status)) {
            Proc->Token = TokenCreate(Proc->PoolType, eProc);
            if(!Proc->Token) {
                Status = STATUS_UNSUCCESSFUL;
            }
        }
    }

    return Status;
}

/*
 *  ProcCreate() -
 *
 *  Allocates and initializes a 'PROC' structure for a given process, 
 *  setting up necessary process-related information including identifiers 
 *  and a security token.
 *
 *  @PoolType: The type of memory pool to be used for allocating the 
 *  'PROC' structure.
 *
 *  @eProc: Pointer to an 'EPROCESS' structure representing the process 
 *  for which the 'PROC' structure will be created and initialized.
 *
 *  Return:
 *    - A pointer to the initialized 'PROC' structure if successful.
 *    - 'NULL' if the allocation or initialization fails.
 */
PPROC ProcCreate(_PoolType_ POOL_TYPE PoolType, _In_ PEPROCESS eProc) {
    
    NTSTATUS Status;
    PPROC Proc;

    Assert(eProc);

    Proc = ProcAlloc(PoolType);
    if(Proc) {
        Status = ProcInit(Proc, eProc); 
        if(!NT_SUCCESS(Status)) {
            ProcDestroy(&Proc);
        }
    }

    return Proc;
}

/*
 *  ProcCopy() -
 *
 *  Copies the contents of a source 'PROC' structure to a destination 'PROC' 
 *  structure. The function ensures that all relevant data, including the 
 *  process image, process identifiers, session ID, and token, are accurately 
 *  copied to the destination.
 *
 *  @Dest: Pointer to the destination 'PROC' structure where the data will be copied.
 *
 *  @Src: Pointer to the source 'PROC' structure from which the data will be copied.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if all data is successfully copied.
 *    - An appropriate 'NTSTATUS' error code if any copy operation fails or 
 *      an exception occurs during the copy process.
 */
NTSTATUS ProcCopy(_Out_ PPROC Dest, _In_ PPROC Src) {
    
    NTSTATUS Status;
    SIZE_T Copied ;

    Copied = 0;
    Status = STATUS_UNSUCCESSFUL;

    if(Dest && Src) {
        IF_SUCCESS(Status,
            TokenCopy(Dest->Token, Src->Token, &Copied); Bytes += Copied,
            CopyToUserMode(Dest->Image, Src->Image, sizeof Src->Image, &Copied); Bytes += Copied,
            CopyToUserMode(&Dest->PPID, &Src->PPID, sizeof Src->PPID , &Copied); Bytes += Copied,
            CopyToUserMode(&Dest->PID , &Src->PID , sizeof Src->PID  , &Copied); Bytes += Copied,
            CopyToUserMode(&Dest->SID , &Src->SID , sizeof Src->SID  , &Copied); Bytes += Copied
        );
    }

    return Status;
}

/*
 *  ProcRef() -
 *
 *  Increments the reference count of a process structure. This function is used
 *  to indicate that a reference to the process structure is being held.
 *
 *  @Proc: A pointer to the process structure whose reference count is to be
 *  incremented.
 */
void ProcRef(_Inout_ PPROC Proc) {

    if(Proc) {
        Proc->Ref++;
    }
}

/*
 *  ProcUnref() -
 *
 *  Decrements the reference count of a process structure and destroys it if
 *  the reference count reaches zero. This function is used to release a
 *  reference to the process structure, and if no more references exist, the
 *  structure is deallocated.
 *
 *  @Proc: A pointer to a pointer to the process structure whose reference
 *  count is to be decremented. If the reference count reaches zero, the
 *  process structure is destroyed and the pointer is set to 'NULL'.
 */
void ProcUnref(_Inout_ PPROC* Proc) {

    if(Proc && *Proc) {
        if((*Proc)->Ref) {
            (*Proc)->Ref--;
            if(!(*Proc)->Ref) {
                ProcDestroy(Proc);
            }
        }
    }
}

/*
 *  ProcDestroy() -
 *
 *  Cleans up and deallocates memory for a given 'PROC' structure. This
 *  function frees the memory allocated for the structure itself and 
 *  then sets the pointer to 'NULL'.
 *
 *  @Proc: Pointer to a pointer to the 'PROC' structure that needs 
 *  to be destroyed.
 */
void ProcDestroy(_Inout_ PPROC* Proc) {

    if(Proc && *Proc) {
        TokenDestroy(&(*Proc)->Token);
        ExFreePoolWithTag(*Proc, 'data');
        *Proc = NULL;
    }
}
