#include "track.h"

void InitTrackerHeap(PTRACKER_HEAP TrackerHeap){
   
    //Ensure Tracker Heap is zeroed
    RtlZeroMemory(TrackerHeap->Targets, sizeof TrackerHeap->Targets);

    //Initialize Uid Counter
    TrackerHeap->UidCounter  = 0;

    KeInitializeSpinLock(&TrackerHeap->Lock);

    return;
}

BOOLEAN AreWeTrackIt(TRACKER_HEAP Heap, HANDLE PID){

    INT i = 0 ;

    while(i < Heap->UidCounter){
        
        if ((Heap->Targets[i].PID) == PID) && (Heap->Targets[i].IsActive))
            return TRUE;

        if (Heap->Targets[i].PID < PID)
            i = ; 
        

    }

}

NTSTATUS InsertTarget(HANDLE PID){

}

NTSTATUS DeactivateTarget(HANDLE PID){

}
