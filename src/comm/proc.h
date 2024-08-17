#ifndef PROC_H
#define PROC_H

/*
 *  PROC -
 *
 *  @Image: Executable's path
 *  @PPID : Parent Process ID
 *  @PID  : Process ID
 *  @SID  : Session ID
 *  @Token: Process tokens
 */
struct _PROC {

    POOL_TYPE PoolType;

    WCHAR  Image[MAX_PATH];
    HANDLE PPID;
    HANDLE PID; 
    HANDLE SID;
    PTOKEN Token;
};

typedef struct _PROC PROC, *PPROC;

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
extern PPROC
ProcCreate(
        _PoolType_ POOL_TYPE PoolType,
        _In_       PEPROCESS eProc
    );

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
extern NTSTATUS
ProcCopy(
        _Out_ PPROC Dest,
        _In_  PPROC Src
    );

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
extern void
ProcDestroy(
        _Inout_ PPROC* Proc
    );

#endif  /* PROC_H */
