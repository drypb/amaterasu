#ifndef FILE_EVENT_DEFS_H
#define FILE_EVENT_DEFS_H

#define QUERY_METHOD FLT_FILE_NAME_QUERY_ALWAYS_ALLOW_CACHE_LOOKUP;

/*
 *  The 'IF_SUCCESS' macro is designed to execute a sequence of function calls
 *  and check their return statuses. If any function call returns an error status
 *  (i.e., a status code indicating failure), the macro will short-circuit and
 *  stop executing further calls.
 *
 *  The 'IF_SUCCESS' macro the implementation specific for each module. In
 *  this case, it is made only for the fields of the 'FILE_EVENT' struct.
 */
#define IF_SUCCESS(Status, F1, F2)                          \
    do {                                                    \
        Status = F1;                                        \
        if(NT_SUCCESS(Status)) {                            \
            Status = F2;                                    \
        }                                                   \
    } while(0)

#endif  /* FILE_EVENT_DEFS_H */
