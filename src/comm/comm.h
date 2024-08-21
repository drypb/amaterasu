#ifndef COMM_H
#define COMM_H

#include <fltKernel.h>
#include <ntstrsafe.h>
#include <suppress.h>
#include <dontuse.h>
#include <ntddk.h>
#include <ntifs.h>
#include <wdm.h>

#include "comm.defs.h"
#include "utils.h"

typedef NTSTATUS (*Copy)(void*, void*);

#endif  /* COMM_H */
