#ifndef COMM_DEFS_H
#define COMM_DEFS_H

#define MAX_DATA        2048
#define MAX_PATH        260
#define MAX_PATH_WCHAR  MAX_PATH * sizeof(WCHAR)
#define MAX_STR_SIZE    NTSTRSAFE_UNICODE_STRING_MAX_CCH * sizeof(WCHAR)

#define _PoolType_      __drv_strictTypeMatch(__drv_typeExpr)

#endif  /* COMM_DEFS_H */
