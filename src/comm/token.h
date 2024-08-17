#ifndef TOKEN_H
#define TOKEN_H

#include "comm.h"

struct _TOKEN {

    PTOKEN_PRIVILEGES Privileges;
    PTOKEN_STATISTICS Statistics;
    PTOKEN_ELEVATION  Elevation;

    DWORD Integrity;
};

typedef struct _TOKEN TOKEN, *PTOKEN;

/*
 *  TokenCreate() -
 *
 *  Allocates and initializes a 'TOKEN' structure for a given process by 
 *  acquiring its primary access token.
 *
 *  @PoolType: Specifies the type of memory pool from which the 'TOKEN' 
 *  structure will be allocated.
 *
 *  @eProc: Pointer to an 'EPROCESS' structure representing the process 
 *  whose primary token will be used to initialize the 'TOKEN' structure.
 *
 *  Return:
 *    - Pointer to the initialized 'TOKEN' structure if successful.
 *    - 'NULL' if the allocation or initialization of the 'TOKEN' 
 *      structure fails.
 */
extern PTOKEN
TokenCreate(
        _PoolType_ POOL_TYPE PoolType,
        _In_       PEPROCESS eProc
    );

/*
 *  TokenCopy() -
 *
 *  Copies the attributes of a source 'TOKEN' structure to a destination 
 *  'TOKEN' structure, ensuring that all relevant token attributes such 
 *  as privileges, statistics, elevation, and integrity level are duplicated.
 *
 *  @Dest: Pointer to the destination user mode 'TOKEN' structure where the 
 *  attributes will be copied.
 *
 *  @Src: Pointer to the source 'TOKEN' structure from which the attributes 
 *  will be copied.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if all token attributes are successfully copied.
 *    - An appropriate 'NTSTATUS' error code if any of the copy 
 *      operations fail.
 */
extern NTSTATUS
TokenCopy(
        _Out_ PTOKEN Dest,
        _In_  PTOKEN Src
    );

/*
 *  TokenDestroy() - 
 *
 *  Destroys a 'TOKEN' structure by deinitializing it and then freeing the
 *  memory allocated for the structure itself.The function will first
 *  deinitialize the token and then free the memory associated with the token.
 *  The pointer is then set to 'NULL' to prevent dangling references.
 *
 *  @Token: Double pointer to the 'TOKEN' structure to be destroyed.
 */
extern void
TokenDestroy(
        _Inout_ PTOKEN* Token
    );

#endif  /* TOKEN_H */
