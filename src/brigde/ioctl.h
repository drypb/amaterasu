#ifndef IOCTL_H
#define IOCTL_H

#include "comm.h"

/*
 *  IoCtlInit() -
 *
 * This function links the driver's object with its device interface and
 * specifies how different types of I/O requests (such as create, close, and
 * device control) should be managed.
 * 
 *  @DriverObj: A pointer to the driver's 'DRIVER_OBJECT' structure, which
 *  represents the driver in the system. The function uses this object to set
 *  up the device and configure the dispatch routines.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the initialization is successful.
 *    - Appropriate 'NTSTATUS' error code if an error occurs during
 *      initialization.
 */
extern NTSTATUS
IoCtlInit(
        _Inout_ PDRIVER_OBJECT DriverObj
    );

/*
 *  IoCtlDeInit() -
 *
 *  This function is responsible for cleaning up and deinitializing the driver
 *  by removing the device and symbolic link created during initialization.
 *
 *  @DriverObj: A pointer to the driver's `DRIVER_OBJECT` structure, which
 *  represents the driver in the system.
 */
extern void
IoCtlDeInit(
        _Inout_ PDRIVER_OBJECT DriverObj
    );

#endif  /* IOCTL_H */
