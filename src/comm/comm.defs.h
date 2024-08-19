#ifndef COMM_DEFS_H
#define COMM_DEFS_H

#define MAX_DATA        2048
#define MAX_PATH        260
#define MAX_PATH_WCHAR  MAX_PATH * sizeof(WCHAR)
#define MAX_STR_SIZE    NTSTRSAFE_UNICODE_STRING_MAX_CCH * sizeof(WCHAR)

#define _PoolType_      __drv_strictTypeMatch(__drv_typeExpr)

#define PP_RSEQ_N()                                         \
         63,62,61,60,                                       \
         59,58,57,56,55,54,53,52,51,50,                     \
         49,48,47,46,45,44,43,42,41,40,                     \
         39,38,37,36,35,34,33,32,31,30,                     \
         29,28,27,26,25,24,23,22,21,20,                     \
         19,18,17,16,15,14,13,12,11,10,                     \
         9,8,7,6,5,4,3,2,1,0


#define PP_ARG_N(                                           \
          _1, _2, _3, _4, _5, _6, _7, _8, _9,_10,           \
         _11,_12,_13,_14,_15,_16,_17,_18,_19,_20,           \
         _21,_22,_23,_24,_25,_26,_27,_28,_29,_30,           \
         _31,_32,_33,_34,_35,_36,_37,_38,_39,_40,           \
         _41,_42,_43,_44,_45,_46,_47,_48,_49,_50,           \
         _51,_52,_53,_54,_55,_56,_57,_58,_59,_60,           \
         _61,_62,_63,N,...) N

#define PP_NARG_(...)                                       \
         PP_ARG_N(__VA_ARGS__)


#define PP_NARG(...)                                        \
         PP_NARG_(__VA_ARGS__,PP_RSEQ_N())


#define IF_SUCCESS2(Status, F1, F2)                         \
    do {                                                    \
        Status = F1;                                        \
        if(NT_SUCCESS(Status)) {                            \
            Status = F2;                                    \
        }                                                   \
    } while(0)


#define IF_SUCCESS3(Status, F1, F2, F3)                     \
    do {                                                    \
        Status = F1;                                        \
        if(NT_SUCCESS(Status)) {                            \
            Status = F2;                                    \
            if(NT_SUCCESS(Status)) {                        \
                Status = F3;                                \
            }                                               \
        }                                                   \
    } while(0)


#define IF_SUCCESS4(Status, F1, F2, F3, F4)                 \
    do {                                                    \
        Status = F1;                                        \
        if(NT_SUCCESS(Status)) {                            \
            Status = F2;                                    \
            if(NT_SUCCESS(Status)) {                        \
                Status = F3;                                \
                if(NT_SUCCESS(Status)) {                    \
                    Status = F4;                            \
                }                                           \
            }                                               \
        }                                                   \
    } while(0)


#define IF_SUCCESS5(Status, F1, F2, F3, F4, F5)             \
    do {                                                    \
        Status = F1;                                        \
        if(NT_SUCCESS(Status)) {                            \
            Status = F2;                                    \
            if(NT_SUCCESS(Status)) {                        \
                Status = F3;                                \
                if(NT_SUCCESS(Status)) {                    \
                    Status = F4;                            \
                    if(NT_SUCCESS(Status)) {                \
                        Status = F5;                        \
                    }                                       \
                }                                           \
            }                                               \
        }                                                   \
    } while(0)


#define IF_SUCCESS_HELPER(Status, N, ...)                   \
    IF_SUCCESS##N(Status, __VA_ARGS__)


#define IF_SUCCESS_(Status, N, ...)                         \
    IF_SUCCESS_HELPER(Status, N, __VA_ARGS__)

/*
 *  IF_SUCCESS() -
 *
 *  The 'IF_SUCCESS' macro is designed to execute a sequence of function calls
 *  and check their return statuses. If any function call returns an error
 *  status (i.e., a status code indicating failure), the macro will short-circuit
 *  and stop executing further calls. The macro determines the number of function
 *  calls provided using the PP_NARG macro and calls the appropriate IF_SUCCESS2,
 *  IF_SUCCESS3, IF_SUCCESS4, etc.
 *
 *  @Status: A variable of type 'NTSTATUS' that stores the status of each
 *  function call. It will hold the status code of the last executed
 *  function or the error code of the first failed function.
 *
 *  @...: A variadic list of function calls that return 'NTSTATUS'. The macro
 *  will execute these calls in sequence as long as each one succeeds.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' if the initialization is successful.
 *    - Appropriate 'NTSTATUS' error code if an error occurs during
 *      initialization.
 */
#define IF_SUCCESS(Status, ...)                             \
    IF_SUCCESS_(Status, __VA_ARGS__)

/*
 *  Debug() -
 *
 *  A macro for logging debug messages. It prints formatted messages to the 
 *  debug output using the DbgPrintEx function with a specific driver identifier 
 *  and error level.
 *
 *  @fmt: The format string for the message.
 *  @...: Additional arguments for the format string.
 */
#define Debug(fmt, ...)			                            \
DbgPrintEx(						                            \
	DPFLTR_IHVDRIVER_ID,			                        \
	DPFLTR_ERROR_LEVEL,				                        \
	fmt"\n",					                            \
    ##__VA_ARGS__						                    \
)

/*
 *  AssertExpr() -
 *
 *  A macro for evaluating an expression and logging a debug message if the 
 *  expression evaluates to false. It prints an error message including the 
 *  function, line number, and the expression that failed.
 *
 *  @expr: The expression to evaluate.
 *
 *  @fmt: The format string for the error message if the expression evaluates
 *  to false.
 *
 *  @...: Additional arguments for the format string.
 */
#define AssertExpr(expr, fmt, ...)							\
((expr)													    \
	? (void)0											    \
	: Debug(												\
		"Assertion failed - %s:%s:%d: "#expr" - "#fmt,	    \
		__func__,										    \
		__LINE__,										    \
       ##__VA_ARGS__						     			\
	)													    \
)

/*
 *  Assert() -
 *
 *  A macro for asserting an expression. It is an alias for AssertExpr,
 *  allowing the use of different formats for assertion messages.
 *
 *  @expr: The expression to evaluate.
 *
 *  @...: Additional arguments for the format string in the assertion
 *  message.
 */
#define Assert(expr, ...) AssertExpr(expr, __VA_ARGS__)

#endif  /* COMM_DEFS_H */
