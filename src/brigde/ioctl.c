
#include "ioctl.h"
#include "ioctl.defs.h"

static UNICODE_STRING DriverName, DeviceSymLink;

/*
 *  A default handler for IRP (I/O Request Packet) major functions that does
 *  not perform any operations. This function simply returns `STATUS_SUCCESS`
 *  as it does not need to handle any specific requests.
 */
static NTSTATUS IoCtlDefaultFunc(_In_ PDEVICE_OBJECT DevObj, _In_ PIRP Irp) {

    UNREFERENCED_PARAMETER(Irp);
    UNREFERENCED_PARAMETER(DeviceObject);

    return STATUS_SUCCESS;
}

/*
 *  IoCtlDevControl() -
 *
 *  @DevObj:
 *  @Irp:
 *
 *  Return:
 *    -
 *    -
 */
static NTSTATUS IoCtlDevControl(_In_ PDEVICE_OBJECT DevObj, _In_ PIRP Irp) {

    NTSTATUS Status;
	PIO_STACK_LOCATION IrpIoStack;
    ULONG IoIndx;
    
    Status = STATUS_UNSUCCESSFUL;

    IrpIoStack = IoGetCurrentIrpStackLocation(Irp);
    if(IrpIoStack) {
        IoIndx = IrpIoStack->Parameters.DeviceIoControl.IoControlCode;
        Status = IoCtlFuncs[IoIndx]();
        if(NT_SUCCESS(Status)) {

        }
    }

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}

/*
 *  IoCtlSetDispatchRoutines() -
 *
 *  Configures the dispatch routines for handling various IRP (I/O Request Packet)
 *  major functions for the driver. This function sets up the driver's major function
 *  pointers to the appropriate handler functions for IRP operations such as creating,
 *  closing, and performing device-specific control operations. This setup allows the
 *  driver to process requests for creating, closing, and controlling the device.
 *
 *  @DriverObj: A pointer to the driver's 'DRIVER_OBJECT' structure, which
 *  represents the driver in the system. The function sets the major function
 *  pointers in this object to the corresponding dispatch routines.
 */
static inline void IoCtlSetDispatchRoutines(_Inout_ PDRIVER_OBJECT DriverObj) {

    Assert(DriverObj);

    DriverObj->MajorFunction[IRP_MJ_CREATE] = IoCtlDefaultFunc;
    DriverObj->MajorFunction[IRP_MJ_CLOSE] = IoCtlDefaultFunc;
    DriverObj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoCtlDevControl;
}

/*
 *  IoCtlDeviceSetup() -
 *
 *  This function sets up the control device for the driver by creating a device
 *  object and a symbolic link that allows user-mode applications to interact
 *  with the driver. It first initializes the device and symbolic link names,
 *  then creates the device object and symbolic link. If both operations succeed,
 *  the created device object is assigned to the driver's device object field.
 *
 *  @DriverObj: A pointer to the driver's 'DRIVER_OBJECT' structure, which
 *  represents the driver's object in the system. This object is used to create
 *  the device and manage interactions between the driver and the operating
 *  system.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if both the device object and symbolic link were
 *      successfully created.
 *    - An appropriate 'NTSTATUS' error code if any step in the setup 
 *      process fails.
 */
static NTSTATUS IoCtlDeviceSetup(_Inout_ PDRIVER_OBJECT DriverObj) {

    NTSTATUS Status;
    PDEVICE_OBJECT Device;

    Assert(DriverObj);

    RtlInitUnicodeString(&DriverName, DRIVER_NAME);
    RtlInitUnicodeString(&DeviceSymLink, DEVICE_SYM_LINK);

    Status = IoCreateDevice(DriverObj, 0, &DriverName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &Device);
    if(NT_SUCCESS(Status)) {
        Status = IoCreateSymbolicLink(&DeviceSymLink, &DriverName);
        if(NT_SUCCESS(Status)) {
            DriverObj->DeviceObject = Device;
        }
    }

    return Status;
}

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
NTSTATUS IoCtlInit(_Inout_ PDRIVER_OBJECT DriverObj) {

    NTSTATUS Status;

    Assert(DriverObj);

    Status = IoCtlDeviceSetup(DriverObj);
    if(NT_SUCCESS(Status)) {
        IoCtlSetDispatchRoutines(DriverObj);
    }

    return Status;
}

/*
 *  IoCtlDeInit() -
 *
 *  This function is responsible for cleaning up and deinitializing the driver
 *  by removing the device and symbolic link created during initialization.
 *
 *  @DriverObj: A pointer to the driver's `DRIVER_OBJECT` structure, which
 *  represents the driver in the system.
 */
void IoCtlDeInit(_Inout PDRIVER_OBJECT DriverObj) {

    Assert(DriverObj);

    IoDeleteDevice(DriverObj->DeviceObject);
    IoDeleteSymbolicLink(&DeviceSymLink);
}
