#include "regfltr.h"
#include "reg-event.h"
#include "regfltr-defs.h"
/*
 * Registry filter structure
 */
PREGFLTR PRegFltr;

/*
 * Callback function that processes registry operations
 * 
 * @CallbackContext: Context passed to the callback function
 * @RegOpClass: The type of registry operation being performed
 * @RegStruct: Pointer to the structure containing operation data
 * 
 */
NTSTATUS RegistryCallback(PVOID CallbackContext, REG_NOTIFY_CLASS RegOpClass, PVOID RegStruct) {
    /*
     * Return error if the registry operation is not tracked in the filter
     */
    if (!PRegFltr->TargetRegOps[RegOpClass])
        return STATUS_UNSUCCESSFUL;

    /*
     * Return error if the operation data structure is not valid
     */
    if (!RegStruct)
        return STATUS_UNSUCCESSFUL;

    return STATUS_SUCCESS; /* Placeholder for successful operation */
}

/*
 * Allocates memory for a registry filter object
 * 
 * @PoolType: The type of memory pool to allocate from
 * 
 * Allocates memory and initializes the registry filter structure.
 */
static PREGFLTR RegFltrAlloc(_PoolType POOL_TYPE PoolType) {
    PREGFLTR RegFltr;

    /*
     * Allocate memory for the registry filter structure
     */
    RegFltr = ExAllocatePool2(PoolType, sizeof(*RegFltr), 'reg');

    /*
     * Initialize the memory if allocation was successful
     */
    if (RegFltr) {
        RtlZeroMemory(RegFltr, sizeof(*RegFltr)); /* Clear the allocated memory */
        RegFltr->PoolType = PoolType; /* Store the memory pool type */
    }

    return RegFltr;
}

/*
 * Set the registry operations to track
 * 
 * @RegFltr: The registry filter object
 * @RegOpCount: Number of registry operations to track
 * 
 * Uses variable arguments to specify which registry operations to track.
 */
static void SetTrackedregOps(PREGFLTR RegFltr, INT RegOpCount, ...) {
    va_list RegOps;

    va_start(RegOps, RegOpCount);

    /*
     * Iterate through the list of registry operations to track
     */
    for (int i = 0; i < RegOpCount; i++) {
        RegFltr->TargetRegOps[va_arg(RegOps, INT)] = 1; /* Mark operation as tracked */
    }

    va_end(RegOps);
    return;
}

/*
 * Initializes the registry filter
 * 
 * @RegFltr: The registry filter object to initialize
 * @DriverObj: The driver object
 * 
 * Initializes the filter structure and registers the callback function.
 */
static NTSTATUS RegFltrInit(_Inout_ PREGFLTR RegFltr, _Inout_ PDRIVER_OBJECT DriverObj) {
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    /*
     * Ensure the filter and driver object are valid
     */
    Assert(RegFltr);
    Assert(DriverObj);

    /*
     * Set function pointers and initialize list
     */
    RegFltr->Copy = RegEventCopy;
    RegFltr->List = ListCreate(RegFltr->PoolType, REGFLTR_MAX, RegEventDestroy);

    /*
     * Set the default registry operations to track
     */
    SetTrackedRegOps(RegFltr, REG_OP_COUNT, TRACKED_REG_OPS);

    /*
     * Register the registry callback if the list was created successfully
     */
    if (RegFltr->List) {
        Status = CmRegisterCallbackEx(
            RegistryCallback,
            REG_CALLBACK_ALTITUDE,
            DriverObj,
            NULL,
            &RegFltr->Cookie,
            NULL
        );
    }

    return Status;
}

/*
 * Loads the registry filter
 * 
 * @DriverObj: The driver object
 * 
 * Allocates and initializes the registry filter, and sets it up for operation.
 */
PREGFLTR RegFltrLoad(_Inout_ PDRIVER_OBJECT DriverObj) {
    NTSTATUS Status;
    PREGFLTR RegFltr;

    /*
     * Ensure the driver object is valid
     */
    Assert(DriverObj);

    /*
     * Allocate and initialize the registry filter
     */
    RegFltr = RegFltrAlloc(POOL_FLAG_NON_PAGED);
    if (RegFltr) {
        PRegFltr = RegFltr;
        Status = RegFltrInit(RegFltr, DriverObj);

        /*
         * If initialization fails, unload the filter
         */
        if (!NT_SUCCESS(Status)) {
            RegFltrUnload(&RegFltr);
        }
    }

    return RegFltr;
}


void RegFltrUnload(_Inout_ PREGFLTR* RegFltr) {

    if(RegFltr && *RegFltr) {
        CmUnregisterCallback((*RegFltr)->Cookie);
        ListDestroy(&(*RegFltr)->List);
        ExFreePoolWithTag((*RegFltr));
        PRegFltr = NULL;
        *RegFltr = NULL;
    }
}

