#ifndef TIME_H
#define TIME_H

#include "comm.h"

struct _TIME {

    TIME_FIELDS TimeFields;
    UINT64      Clock;
};

typedef struct _TIME TIME, *PTIME;

/*
 *  TimeInit() -
 *
 *  Initializes a 'TIME' structure by setting its 'Clock' field to the 
 *  current processor time stamp counter value and initializing its time 
 *  fields using 'InitTimeFields'.
 *
 *  @Time: Pointer to the 'TIME' structure that will be initialized.
 */
extern void
TimeInit(
        _Inout_ PTIME Time
    );

/*
 *  TimeCopy() -
 *
 *  Copies the time data from a source 'TIME' structure to a destination 
 *  'TIME' structure, including both the time fields and the clock value.
 *  The copy operation ensures that all time-related information is 
 *  transferred accurately to the user mode.
 *
 *  @Dest: Pointer to the destination 'TIME' structure where the time data 
 *  will be copied.
 *
 *  @Src: Pointer to the source 'TIME' structure from which the time data 
 *  will be copied.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the time data is successfully copied.
 *    - 'STATUS_UNSUCCESSFUL' if an exception occurs during the copy 
 *      operation or if the pointers are invalid.
 */
extern NTSTATUS
TimeCopy(
        _Out_ PTIME Dest,
        _In_  PTIME Src
    );

#endif  /* TIME_H */ 
