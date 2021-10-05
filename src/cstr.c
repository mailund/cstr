#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// Here we make sure that we emit the inline
// functions in the header.
#define INLINE extern inline
#include "cstr.h"

#include "cstr_internal.h"

bool cstr_sslice_eq(struct cstr_sslice x,
                    struct cstr_sslice y)
{
    if (x.len != y.len)
        return false;

    for (size_t i = 0; i < x.len; i++)
    {
        if (x.buf[i] != y.buf[i])
            return false;
    }

    return true;
}
