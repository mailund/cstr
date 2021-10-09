#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// Here we make sure that we emit the inline
// functions in the header.
#define INLINE extern inline
#include "cstr.h"

#include "cstr_internal.h"

#define ASSERT_BUFFER_SIZE_HEADER(HEADER_SIZE, OBJ_SIZE, LEN)                 \
if ((SIZE_MAX - (HEADER)) / (OBJ_SIZE) < (LEN))                           \
{                                                                         \
fprintf(stderr, "Trying to allocte a buffer longer than SIZE_MAX\n"); \
exit(2);                                                              \
}
#define ASSERT_BUFFER_SIZE(OBJ_SIZE, LEN) \
ASSERT_BUFFER_SIZE_HEADER(0, OBJ_SIZE, LEN)

void *cstr_malloc(size_t size)
{
    void *buf = malloc(size);
    if (!buf)
    {
        fprintf(stderr, "Allocation error, terminating\n");
        exit(2);
    }
    return buf;
}

void *cstr_malloc_flex_array(size_t base_size,
                             size_t elm_size,
                             size_t len)
{
    if ((SIZE_MAX - base_size) / elm_size < len) {
        fprintf(stderr, "Trying to allocte a buffer longer than SIZE_MAX\n");
        exit(2);
    }
    return cstr_malloc(base_size + elm_size * len);
}

void *cstr_malloc_buffer(size_t obj_size, size_t len)
{
    // a buffer is just a flexible array in a struct that
    // has zero header...
    return cstr_malloc_flex_array(0, obj_size, len);
}


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
