#ifndef COMM_DEFS_H
#define COMM_DEFS_H

#define MAX_PATH        260
#define MAX_DATA        2048
#define MAX_STR_SIZE    NTSTRSAFE_UNICODE_STRING_MAX_CCH * sizeof(WCHAR)

#define _PoolType_ __drv_strictTypeMatch(__drv_typeExpr)

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
#define Debug(fmt, ...)			                                                        	\
DbgPrintEx(						                                                        	\
	DPFLTR_IHVDRIVER_ID,			                                                        \
	DPFLTR_ERROR_LEVEL,				                                                        \
	fmt"\n",					                                                        	\
    ##__VA_ARGS__						                                                    \
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
#define AssertExpr(expr, fmt, ...)							                                \
((expr)													                                	\
	? (void)0											                                	\
	: Debug(												                                \
		"Assertion failed - %s:%s:%d: "#expr" - "fmt,	                                	\
		__func__,										                                	\
		__LINE__,										                                	\
       ##__VA_ARGS__						     			                                \
	)														                                \
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
