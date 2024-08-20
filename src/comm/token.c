
#include "token.h"
#include "token.defs.h"

/*
 *  TokenAlloc() - 
 *
 *  Allocate a 'TOKEN' structure.
 *
 *  @PoolType: The type of memory pool to allocate from 
 *  (paged or nonpaged).
 *
 *  Return:
 *    - Pointer to the allocated 'TOKEN' structure on success.
 *    - 'NULL' if memory allocation fails.
 */
static PTOKEN TokenAlloc(_PoolType_ POOL_TYPE PoolType) {

    PTOKEN Token;

    Token = ExAllocatePool2(PoolType, sizeof *Token, 'tken');
    if(!Token) {
        return NULL;
    }

    RtlZeroMemory(Token, sizeof *Token);

    /*
     *  Store the pool type used for the allocation in 'Token' to ensure 
     *  correct memory handling, regardless of whether the caller routine is 
     *  paged or nonpaged.
     */
    Token->PoolType = PoolType;

    return Token;
}

/*
 *  AcquirePrimaryToken() - 
 *
 *  Acquires the primary access token for a given process.
 *
 *  @pToken: Pointer to a 'PACCESS_TOKEN' pointer that will receive 
 *  the process's primary token. If the token is successfully 
 *  acquired, this pointer will be set to the token.
 *
 *  @eProc: Pointer to the 'EPROCESS' structure representing the 
 *  process for which the token is acquired.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the primary token is successfully 
 *      acquired and referenced.
 *    - 'STATUS_UNSUCCESSFUL' if the acquisition fails or the 
 *      'pToken' pointer is 'NULL'.
 */
static inline NTSTATUS AcquirePrimaryToken(_Out_ PACCESS_TOKEN* pToken, _In_ PEPROCESS eProc) {

    Assert(eProc);

    if(pToken) {
        *pToken = PsReferencePrimaryToken(eProc);
        if(*pToken) {
            return STATUS_SUCCESS;
        }
    }

    return STATUS_UNSUCCESSFUL;
}

/*
 *  TokenInitAttrs() - 
 *
 *  Initializes the attributes of a 'TOKEN' structure by querying 
 *  information from a given access token.
 *
 *  @Token: Pointer to the 'TOKEN' structure to be initialized 
 *  with attributes.
 *
 *  @pToken: Pointer to an access token ('PACCESS_TOKEN') from which
 *  the attributes will be queried.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if all token attributes are successfully 
 *       queried and initialized.
 *    - An appropriate 'NTSTATUS' error code if any of the attribute 
 *      queries fail.
 */
static NTSTATUS TokenInitAttrs(_Inout_ PTOKEN Token, _In_ PACCESS_TOKEN pToken) {

    NTSTATUS Status;

    Assert(Token);
    Assert(pToken);

    /*
     *  'SeQueryInformationToken' returns a  pointer to a location that
     *  contains the address of a buffer that holds the requested information.
     *  The buffer is allocated by 'SeQueryInformationToken' from paged pool
     *  and must eventually be freed by the caller by using 'ExFreePool'.
     */
    IF_SUCCESS(Status,
        SeQueryInformationToken(pToken, TokenPrivileges, &Token->Privileges),
        SeQueryInformationToken(pToken, TokenStatistics, &Token->Statistics),
        SeQueryInformationToken(pToken, TokenElevation, &Token->Elevation),
        SeQueryInformationToken(pToken, TokenIntegrityLevel, &Token->Integrity)
    );

    return Status;
}

/*
 *  TokenInit() -
 *
 *  Initializes a 'TOKEN' structure by acquiring the primary access 
 *  token of a specified process and populating the 'TOKEN' structure 
 *  with its attributes.
 *
 *  @Token: Pointer to the 'TOKEN' structure that will be initialized 
 *  with attributes from the process's primary token.
 *
 *  @eProc: Pointer to an 'EPROCESS' structure representing the process 
 *  whose primary token will be used to initialize the 'TOKEN' structure.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the token attributes are successfully 
 *      initialized.
 *    - An appropriate 'NTSTATUS' error code if acquiring the primary 
 *      token or initializing the attributes fails.
 */
static NTSTATUS TokenInit(_Inout_ PTOKEN Token, _In_ PEPROCESS eProc) {

    NTSTATUS Status;
    PACCESS_TOKEN pToken;

    Assert(Token);
    Assert(eProc);

    pToken = NULL;
    Status = AcquirePrimaryToken(&pToken, eProc);
    if(NT_SUCCESS(Status)) {
        Status = TokenInitAttrs(Token, pToken);
    }

    if(pToken) {
        PsDereferencePrimaryToken(pToken);
    }

    return Status;
}

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
PTOKEN TokenCreate(_PoolType_ POOL_TYPE PoolType, _In_ PEPROCESS eProc) {

    NTSTATUS Status;
    PTOKEN Token;
    
    Assert(eProc);

    Token = TokenAlloc(PoolType);
    if(Token) {
        Status = TokenInit(Token, eProc);
        if(!NT_SUCCESS(Status)) {
            TokenDestroy(&Token);
        }
    }

    return Token;
}

/*
 *  TokenUpdate() -
 *
 *  Updates the attributes of a 'TOKEN' structure using information from 
 *  the specified process. This function reinitializes the token by calling 
 *  'TokenInit' to refresh its attributes based on the provided process.
 *
 *  @Token: Pointer to the 'TOKEN' structure that needs to be updated with 
 *  new attributes.
 *
 *  @eProc: Pointer to the 'EPROCESS' structure representing the process 
 *  from which new token information will be obtained.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the token attributes are successfully updated.
 *    - An appropriate 'NTSTATUS' error code if the update fails or an exception 
 *      occurs during the initialization.
 */
NTSTATUS TokenUpdate(_Inout_ PTOKEN Token, _In_ PEPROCESS eProc) {

    Assert(Token);
    Assert(eProc);

    return TokenInit(Token, eProc);
}

/*
 *  TokenCopy() -
 *
 *  Copies the attributes of a source 'TOKEN' structure to a destination 
 *  'TOKEN' structure, ensuring that all relevant token attributes such 
 *  as privileges, statistics, elevation, and integrity level are duplicated.
 *
 *  @Dest: Pointer to the destination 'TOKEN' structure where the attributes 
 *  will be copied.
 *
 *  @Src: Pointer to the source 'TOKEN' structure from which the attributes 
 *  will be copied.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if all token attributes are successfully copied.
 *    - An appropriate 'NTSTATUS' error code if any of the copy 
 *      operations fail.
 */
NTSTATUS TokenCopy(_Out_ PTOKEN Dest, _In_  PTOKEN Src) {

    NTSTATUS Status;

    Status = STATUS_UNSUCCESSFUL;
    if(Dest && Src) {
        IF_SUCCESS(Status,
            CopyToUserMode(Dest->Privileges, Src->Privileges, sizeof Src->Privileges, TOKEN_PRIVILEGES),
            CopyToUserMode(Dest->Statistics, Src->Statistics, sizeof Src->Statistics, TOKEN_STATISTICS),
            CopyToUserMode(Dest->Elevation , Src->Elevation , sizeof Src->Elevation , TOKEN_ELEVATION),
            CopyToUserMode(&Dest->Integrity, &Src->Integrity, sizeof Src->Integrity , DWORD)
        );
    }

    return Status;
}

/*
 *  TokenDeInit() - 
 *
 *  Deinitializes a 'TOKEN' structure by freeing any allocated memory for its
 *  attributes and clearing the structure's contents.The function will free
 *  memory associated with the token's privileges, statistics, and elevation
 *  attributes, and then zero out the entire structure.

 *  @Token: Pointer to the 'TOKEN' structure to be deinitialized.  
 */
static inline void TokenDeInit(_Inout_ PTOKEN Token) {

    ExFreePool(Token->Privileges); 
    ExFreePool(Token->Statistics);
    ExFreePool(Token->Elevation);
    RtlZeroMemory(Token, sizeof *Token);
}

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
void TokenDestroy(_Inout_ PTOKEN* Token) {

    if(Token && *Token) {
        TokenDeInit(*Token);
        ExFreePoolWithTag(*Token, 'tken');
        *Token = NULL;
    }
}
