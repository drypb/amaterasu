#ifndef FSFLTR_H
#define FSFLTR_H

#include "comm.h"

struct _FSFLTR {

    PDRIVER_OBJECT DriverObj;
    PFLT_FILTER    FilterHandle;
    COPY           Copy;
    LIST           List;
};

typedef struct _FSFLTR FSFLTR, *PFSFLTR;

/*
 *  FSFltrLoad() -
 *
 *  Loads and initializes a file system filter associated with the driver. This
 *  function allocates memory for the filter, initializes it, and returns a
 *  pointer to the filter structure.
 *
 *  @DriverObj: A pointer to the driver's 'DRIVER_OBJECT' structure, which
 *  represents the driver in the system. This object is used during the
 *  initialization of the file system filter.
 *
 *  Return:
 *    - A pointer to the initialized `FSFLTR` structure if successful.
 *    - 'NULL' if the filter could not be loaded or initialized.
 */
extern PFSFLTR
FSFltrLoad(
        _Inout_ PDRIVER_OBJECT DriverObj
    );

/*
 *  FSFltrUnload() -
 *
 *  Unloads the file system filter by unregistering it and releasing any
 *  associated resources. This function ensures that the filter is properly
 *  unregistered and frees the memory allocated for the filter structure.
 *
 *  @FSFltr: A double pointer to the `FSFLTR` structure representing the file
 *  system filter. If the filter is valid, it is unregistered and the memory
 *  associated with it is freed.
 */
extern void
FSFltrUnload(
        _Inout_ PFSFLTR* FSFltr
    );

#endif  /* FSFLTR_H */
