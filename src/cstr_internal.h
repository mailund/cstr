#ifndef CSTR_INTERNAL_H
#define CSTR_INTERNAL_H

#include "cstr.h"

// get config and unit test prototypes if we make them
#include "config.h"
#include "unittests.h"

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

// if we just want to jump to error handling but not set err because
// it is already set.
#define reraise_error_if(EXPR)                                                 \
    do {                                                                       \
        if (EXPR)                                                              \
            goto error;                                                        \
    } while (0)

// shorten names a bit for the internal code...
typedef enum cstr_errcodes errcodes;
typedef struct cstr_alphabet alpha;
typedef struct cstr_str_slice sslice;

// set pointers to 0 when we free them
#define free_and_null(p)                                                       \
    do {                                                                       \
        free(p);                                                               \
        p = 0;                                                                 \
    } while (0)

#endif
