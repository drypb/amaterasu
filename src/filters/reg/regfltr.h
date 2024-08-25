#ifndef REGFLTR_H
#define REGFLTR_H

#include "comm.h"

DECLARE_CONST_UNICODE_STRING(REG_CALLBACK_ALTITUDE, L"409900");

struct _REGFLTR {

    POOL_TYPE      PoolType;
    PDRIVER_OBJECT DriverObj;
    LARGE_INTERGER Cookie;
    BOOLEAN        TargetRegOps[MaxRegNtNotifyClass];     
    COPY           Copy;
    PLIST          List;

};

typedef struct _REGFLTR REGFLTR, *PREGFLTR;

extern PREGFLTR PRegFltr; 

extern PREGFLTR
RegFltrLoad(
        _Inout_ PDRIVER_OBJECT DriverObj
);


extern void
RegFltrUnload(
        _Inout_ PREGFLTR* RegFltr
);

#endif
