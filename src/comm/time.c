
#include "time.h"
#include "time.defs.h"

/*
 *  InitTimeFields() -
 *
 *  Initializes a 'TIME_FIELDS' structure by querying the current system time 
 *  and converting it to local time. The local time is then converted into 
 *  time fields and stored in the 'TIME_FIELDS' structure.
 *
 *  @TimeFields: Pointer to the 'TIME_FIELDS' structure that will be 
 *  initialized with the current local time fields.
 */
static inline void InitTimeFields(_Out_ PTIME_FIELDS TimeFields) {

    LARGE_INTEGER SysTime;
    LARGE_INTEGER LocalTime;

    Assert(TimeFields);

    KeQuerySystemTime(&SysTime);
    ExSystemTimeToLocalTime(&SysTime, &LocalTime);
    RtlTimeToTimeFields(&LocalTime, TimeFields);
}

/*
 *  TimeInit() -
 *
 *  Initializes a 'TIME' structure by setting its 'Clock' field to the 
 *  current processor time stamp counter value and initializing its time 
 *  fields using 'InitTimeFields'.
 *
 *  @Time: Pointer to the 'TIME' structure that will be initialized.
 */
void TimeInit(_Inout_ PTIME Time) {

    Assert(Time);

    Time->Clock = __rdtsc();
    InitTimeFields(&Time->TimeFields);
}

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
NTSTATUS TimeCopy(_Out_ PTIME Dest, _In_ PTIME Src) {

    NTSTATUS Status;

    if(Dest && Src) {
        IF_SUCCESS(Status,
            CopyToUserMode(&Dest->TimeFields, &Src->TimeFields, sizeof Src->TimeFields),
            CopyToUserMode(&Dest->Clock, &Src->Clock, sizeof Src->Clock)
        );
    }

    return Status;
}
