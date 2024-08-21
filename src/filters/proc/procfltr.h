#ifndef PROCFLTR_H
#define PROCFLTR_H

#include "comm.h"

struct _PROCFLTR {

    POOL_TYPE      PoolType;
    PDRIVER_OBJECT DriverObj;
    PLIST          List;
    Copy           Copy;
};

typedef struct _PROCFLTR PROCFLTR, *PPROCFLTR;

/*
 * ProcFltrLoad() -
 *
 * Loads and initializes a process filter associated with the driver. This
 * function allocates memory for the process filter, initializes it, and
 * returns a pointer to the filter structure.
 *
 * @DriverObj: A pointer to the driver's 'DRIVER_OBJECT' structure, which
 * represents the driver in the system. This object is used during the
 * initialization of the process filter.
 *
 * Return:
 *    - A pointer to the initialized 'PROCFLTR' structure if successful.
 *    - 'NULL' if the filter could not be allocated or initialized.
 */
extern PPROCFLTR
ProcFltrLoad(
        _Inout_ PDRIVER_OBJECT DriverObj
    );


/*
 *  ProcFltrUnload() -
 *
 *  @ProcFltr:
 */
extern void
ProcFltrUnload(
        _Inout_ PPROCFLTR* ProcFltr
    );

#endif  /* PROCFLTR_H */
