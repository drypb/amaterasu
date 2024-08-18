#ifndef TOKEN_DEFS_H
#define TOKEN_DEFS_H

/*
 *  The 'IF_SUCCESS' macro is designed to execute a sequence of function calls
 *  and check their return statuses. If any function call returns an error status
 *  (i.e., a status code indicating failure), the macro will short-circuit and
 *  stop executing further calls.
 *
 *  The 'IF_SUCCESS' macro has a specific implementation for each module. In
 *  this case, it is made only for the fields of the 'TOKEN' struct.
 */
#define IF_SUCCESS(Status, F1, F2, F3, F4)                  \
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

#endif  /* TOKEN_DEFS_H */
