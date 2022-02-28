#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// Here we make sure that we emit the inline
// functions in the header.
#define INLINE extern inline
#include "cstr.h"

long long cstr_strlen(const char *x)
{
    size_t n = strlen(x) * sizeof(uint8_t); // Flawfinder: ignore -- x should be \0 terminated here
    assert(n <= LLONG_MAX);
    return (long long)n;
}

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

void *cstr_malloc_header_array(size_t base_size,
                               size_t elm_size,
                               size_t len)
{
    if ((SIZE_MAX - base_size) / elm_size < len)
    {
        fprintf(stderr, "Trying to allocte a buffer longer than SIZE_MAX\n");
#ifndef __clang_analyzer__
        exit(2);
#endif
    }
    return cstr_malloc(base_size + elm_size * len);
}

void *cstr_malloc_buffer(size_t obj_size, size_t len)
{
    // a buffer is just a flexible array in a struct that
    // has zero header...
    return cstr_malloc_header_array(0, obj_size, len);
}

#define GEN_ALLOC_SLICE(STYPE, QUAL, TYPE)                       \
    cstr_##STYPE *cstr_alloc_##STYPE(long long len)              \
    {                                                            \
        struct                                                   \
        {                                                        \
            cstr_##STYPE slice;                                  \
            QUAL TYPE data[];                                    \
        } *buf = CSTR_MALLOC_FLEX_ARRAY(buf, data, (size_t)len); \
        buf->slice.len = len;                                    \
        buf->slice.buf = buf->data;                              \
        return (cstr_##STYPE *)buf;                              \
    }
GEN_ALLOC_SLICE(sslice, , uint8_t)
GEN_ALLOC_SLICE(const_sslice, const, uint8_t)
GEN_ALLOC_SLICE(islice, , int)
GEN_ALLOC_SLICE(const_islice, const, int)
GEN_ALLOC_SLICE(uislice, , unsigned int)
GEN_ALLOC_SLICE(const_uislice, const, unsigned int)

#define GEN_SLICE_EQ(STYPE)                   \
    bool cstr_eq_##STYPE(cstr_##STYPE x,      \
                         cstr_##STYPE y)      \
    {                                         \
        if (x.len != y.len)                   \
            return false;                     \
                                              \
        for (long long i = 0; i < x.len; i++) \
        {                                     \
            if (x.buf[i] != y.buf[i])         \
                return false;                 \
        }                                     \
                                              \
        return true;                          \
    }
GEN_SLICE_EQ(sslice)
GEN_SLICE_EQ(const_sslice)
GEN_SLICE_EQ(islice)
GEN_SLICE_EQ(const_islice)
GEN_SLICE_EQ(uislice)
GEN_SLICE_EQ(const_uislice)

#define GEN_SLICE_LE(STYPE)                            \
    bool cstr_le_##STYPE(cstr_##STYPE x,               \
                         cstr_##STYPE y)               \
    {                                                  \
        long long n = (x.len < y.len) ? x.len : y.len; \
        for (long long i = 0; i < n; i++)              \
        {                                              \
            if (x.buf[i] < y.buf[i])                   \
                return true;                           \
            if (x.buf[i] > y.buf[i])                   \
                return false;                          \
        }                                              \
                                                       \
        return x.len <= y.len;                         \
    }
GEN_SLICE_LE(sslice)
GEN_SLICE_LE(const_sslice)
GEN_SLICE_LE(islice)
GEN_SLICE_LE(const_islice)
GEN_SLICE_LE(uislice)
GEN_SLICE_LE(const_uislice)

#define GEN_SLICE_GE(STYPE)                            \
    bool cstr_ge_##STYPE(cstr_##STYPE x,               \
                         cstr_##STYPE y)               \
    {                                                  \
        long long n = (x.len < y.len) ? x.len : y.len; \
        for (long long i = 0; i < n; i++)              \
        {                                              \
            if (x.buf[i] < y.buf[i])                   \
                return false;                          \
            if (x.buf[i] > y.buf[i])                   \
                return true;                           \
        }                                              \
                                                       \
        return x.len >= y.len;                         \
    }
GEN_SLICE_GE(sslice)
GEN_SLICE_GE(const_sslice)
GEN_SLICE_GE(islice)
GEN_SLICE_GE(const_islice)
GEN_SLICE_GE(uislice)
GEN_SLICE_GE(const_uislice)

#define GEN_SLICE_LCP(STYPE)                           \
    long long cstr_lcp_##STYPE(cstr_##STYPE x,         \
                               cstr_##STYPE y)         \
    {                                                  \
        long long n = (x.len < y.len) ? x.len : y.len; \
                                                       \
        for (long long i = 0; i < n; i++)              \
        {                                              \
            if (x.buf[i] != y.buf[i])                  \
                return i;                              \
        }                                              \
                                                       \
        return n;                                      \
    }
GEN_SLICE_LCP(sslice)
GEN_SLICE_LCP(const_sslice)
GEN_SLICE_LCP(islice)
GEN_SLICE_LCP(const_islice)
GEN_SLICE_LCP(uislice)
GEN_SLICE_LCP(const_uislice)

#define GEN_FPRINT_SLICE(STYPE, FMT)                                      \
    void cstr_fprint_##STYPE(FILE *f, cstr_##STYPE x)                     \
    {                                                                     \
        fprintf(f, "[");                                                  \
        char *sep = "";                                                   \
        for (int i = 0; i < x.len; i++)                                   \
        {                                                                 \
            fprintf(f, "%s" FMT, sep, x.buf[i]); /* Flawfinder: ignore */ \
            sep = ", ";                                                   \
        }                                                                 \
        fprintf(f, "]");                                                  \
    }
GEN_FPRINT_SLICE(sslice, "%c")
GEN_FPRINT_SLICE(const_sslice, "%c")
GEN_FPRINT_SLICE(islice, "%d")
GEN_FPRINT_SLICE(const_islice, "%d")
GEN_FPRINT_SLICE(uislice, "%u")
GEN_FPRINT_SLICE(const_uislice, "%u")
