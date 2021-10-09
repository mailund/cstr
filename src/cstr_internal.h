#ifndef CSTR_INTERNAL_H
#define CSTR_INTERNAL_H

#include "cstr.h"

// get config and unit test prototypes if we make them
#include "config.h"
#include "unittests.h"

// shorten names a bit for the internal code...
typedef struct cstr_alphabet alpha;
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
