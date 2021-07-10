#ifndef CSTR_INTERNAL_H
#define CSTR_INTERNAL_H

#include "cstr.h"

#define set_error(CODE)                                                        \
    do {                                                                       \
        if (err)                                                               \
            *err = (CODE);                                                     \
    } while (0)
#define clear_error() set_error(CSTR_NO_ERROR)

#define goto_if(EXPR, LABEL, CODE)                                             \
    do {                                                                       \
        if (EXPR) {                                                            \
            set_error(CODE);                                                   \
            goto LABEL;                                                        \
        }                                                                      \
    } while (0)

#define error_if(EXPR, CODE) goto_if(EXPR, error, CODE)
#define alloc_error_if(EXPR) error_if(EXPR, CSTR_ALLOCATION_ERROR)

// shorten names a bit for the internal code...
typedef enum cstr_errcodes errcodes;
typedef struct cstr_alphabet alpha;
typedef struct cstr_str_slice sslice;

#endif
