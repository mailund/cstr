#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// Here we make sure that we emit the inline
// functions in the header.
#define INLINE extern inline
#include "cstr.h"

#include "cstr_internal.h"

bool cstr_alloc_sslice_buffer(sslice* slice, size_t len)
{
    assert(slice->buf == 0); // This is to avoid accidental leak.
    void* buf = malloc(len + 1);
    if (buf) {
        sslice new_slice = CSTR_SSLICE(buf, len);
        memcpy(slice, &new_slice, sizeof *slice);
        return true;
    }
    return false;
}

void cstr_free_sslice_buffer(sslice* slice)
{
    free(slice->buf);
    static sslice nil = CSTR_NIL_SLICE;
    memcpy(slice, &nil, sizeof *slice);
}

bool cstr_alloc_islice_buffer(islice* slice, size_t len)
{
    assert(slice->buf == 0); // This is to avoid accidental leak.
    void* buf = malloc(len * sizeof(*slice->buf));
    if (buf) {
        islice new_slice = CSTR_ISLICE(buf, len);
        memcpy(slice, &new_slice, sizeof *slice);
        return true;
    }
    return false;
}

void cstr_free_islice_buffer(islice* slice)
{
    free(slice->buf);
    islice nil = CSTR_NIL_SLICE;
    memcpy(slice, &nil, sizeof *slice);
}
