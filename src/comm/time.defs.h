#ifndef TIME_DEFS_H
#define TIME_DEFS_H

/*
 *  The 'IF_SUCCESS' macro is designed to execute a sequence of function calls
 *  and check their return statuses. If any function call returns an error status
 *  (i.e., a status code indicating failure), the macro will short-circuit and
 *  stop executing further calls.
 *
 *  The 'IF_SUCCESS' macro is implementation specific for each module. In
 *  this case, it is made only for the fields of the 'TIME' struct.
 */
#define IF_SUCCESS(Status, F1, F2)                          \
    do {                                                    \
        Status = F1;                                        \
        if(NT_SUCCESS(Status)) {                            \
            Status = F2;                                    \
        }                                                   \
    } while(0)

#endif  /* TIME_DEFS_H */
