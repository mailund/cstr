#ifndef CSTR_INTERNAL_H
#define CSTR_INTERNAL_H

#include "cstr.h"

// get config and unit test prototypes if we make them
#include "config.h"
#include "unittests.h"

#define set_error(CODE)    \
    do                     \
    {                      \
        if (err)           \
            *err = (CODE); \
    } while (0)
#define clear_error() set_error(CSTR_NO_ERROR)

// If EXPR is false-y, set the error code and jump
// to LABEL
#define error_goto_if(EXPR, LABEL, CODE) \
    do                                   \
    {                                    \
        if (EXPR)                        \
        {                                \
            set_error(CODE);             \
            goto LABEL;                  \
        }                                \
    } while (0)

#define alloc_error_if(EXPR, LABEL) error_goto_if(EXPR, LABEL, CSTR_ALLOCATION_ERROR)
#define mapping_error_if(EXPR, LABEL) error_goto_if(EXPR, LABEL, CSTR_MAPPING_ERROR)
#define size_error_if(EXPR, LABEL) error_goto_if(EXPR, LABEL, CSTR_SIZE_ERROR)

// if we just want to jump to error handling but not set err because
// it is already set.
#define reraise_error_if(EXPR, LABEL) \
    do                                \
    {                                 \
        if (EXPR)                     \
            goto LABEL;               \
    } while (0)

// We allocate a lot, so convinience macro for that
#define try_alloc(LABEL, EXPR) \
    alloc_error_if(!(EXPR), LABEL)
#define try_alloc_flag(LABEL, FLAG, EXPR) \
    alloc_error_if(!(FLAG = !!(EXPR)), LABEL)
// Re-raise an existing error if EXPR evaluates to false-y.
#define try_reraise(LABEL, EXPR) \
    reraise_error_if(!(EXPR), LABEL)
#define try_reraise_flag(LABEL, FLAG, EXPR) \
    reraise_error_if(!(FLAG = !!(EXPR)), LABEL)

// shorten names a bit for the internal code...
typedef enum cstr_errcodes errcodes;
typedef struct cstr_alphabet alpha;
typedef struct cstr_const_sslice csslice;
typedef struct cstr_sslice sslice;
typedef struct cstr_islice islice;

// set pointers to 0 when we free them
#define free_and_null(p) \
    do                   \
    {                    \
        free(p);         \
        p = 0;           \
    } while (0)

#endif
