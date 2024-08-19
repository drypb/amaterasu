#ifndef IOCTL_DEFS_H
#define IOCTL_DEFS_H

#define DRIVER_NAME     L"\\Device\\MiniFilter"
#define DEVICE_SYM_LINK L"\\DosDevices\\MiniFilter"

#define IOCTL_GET_FS_EVENT      CTL_CODE()
#define IOCTL_GET_PROC_EVENT    CTL_CODE()
#define IOCTL_GET_REG_EVENT     CTL_CODE()
#define IOCTL_GET_HANDLE_EVENT  CTL_CODE()

#endif  /* IOCTL_DEFS_H */
