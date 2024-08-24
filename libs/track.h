#ifndef TRACK_H
#define TRACK_H

#define MAX_TRACK 1024

struct _TargetID{
    ULONG PID;
    ULONG UID;
    BOOLEAN IsActive;
};

typedef struct _TargetID TARGET_ID, *PTARGET_ID;

struct _TRACKER_HEAP{
    TARGET_ID Targets[MAX_TRACK];
    ULONG UidCounter;
    KSPIN_LOCK Lock;
};

typedef struct _TRACKER_HEAP TRACKER_HEAP, *PTRACKER_HEAP;

extern void
InitTrackerHeap(
    TRACKER_HEAP TrackerHeap    
);

extern BOOLEAN
ShouldWeTrackIt(
    HANDLE PPID,
    HANDLE PID 
);

extern NTSTATUS
InsertTarget(
    HANDLE PID
);

extern NTSTATUS
DeactivateTarget(
    HANDLE PID
);

#endif 
