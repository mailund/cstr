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

void *cstr_realloc(void *p, size_t size)
{
    void *buf = realloc(p, size);
    if (!buf)
    {
        fprintf(stderr, "Allocation error, terminating\n");
        exit(2);
    }
    return buf;
}

void *cstr_malloc(size_t size)
{
    return cstr_realloc(0, size);
}

void *cstr_realloc_header_array(void *p,
                                size_t base_size,
                                size_t elm_size,
                                size_t len)
{
    if ((SIZE_MAX - base_size) / elm_size < len)
    {
        fprintf(stderr, "Trying to allocte a buffer longer than SIZE_MAX\n");
        exit(2);
    }
    return cstr_realloc(p, base_size + elm_size * len);
}

void *cstr_malloc_header_array(size_t base_size,
                               size_t elm_size,
                               size_t len)
{
    return cstr_realloc_header_array(0, base_size, elm_size, len);
}

void *cstr_realloc_buffer(void *p, size_t obj_size, size_t len)
{
    // a buffer is just a flexible array in a struct that
    // has zero header...
    return cstr_realloc_header_array(p, 0, obj_size, len);
}

void *cstr_malloc_buffer(size_t obj_size, size_t len)
{
    // a buffer is just a flexible array in a struct that
    // has zero header...
    return cstr_malloc_header_array(0, obj_size, len);
}

#define GEN_ALLOC_SLICE_BUF(STYPE, QUAL, TYPE)                                    \
    cstr_##STYPE##_buf *cstr_alloc_##STYPE##_buf(long long len, long long cap)    \
    {                                                                             \
        /* Realloc doesn't deal well with cap 0 so make it always at least 1 */   \
        cap = (cap > 0) ? cap : 1;                                                \
        cstr_##STYPE##_buf *buf = CSTR_MALLOC_FLEX_ARRAY(buf, data, (size_t)cap); \
        buf->slice.len = len;                                                     \
        buf->slice.buf = buf->data;                                               \
        buf->cap = cap;                                                           \
        return buf;                                                               \
    }

#define GEN_APPEND_SLICE_BUF(STYPE, QUAL, TYPE)                                                            \
    cstr_##STYPE##_buf_slice cstr_append_##STYPE##_buf(cstr_##STYPE##_buf **buf, TYPE val)                 \
    {                                                                                                      \
        if ((*buf)->slice.len == (*buf)->cap)                                                              \
        {                                                                                                  \
            (*buf)->cap *= 2;                                                                              \
            *buf = cstr_realloc_header_array(*buf, offsetof(struct cstr_##STYPE##_buf, data),              \
                                             sizeof(*buf)->data[0], (size_t)(*buf)->cap);                  \
            (*buf)->slice.buf = (*buf)->data;                                                              \
        }                                                                                                  \
        (*buf)->slice.buf[(*buf)->slice.len++] = val;                                                      \
        return (cstr_##STYPE##_buf_slice){.buf = buf, .from = 0, .to = (*buf)->slice.len};                 \
    }                                                                                                      \
    cstr_##STYPE##_buf_slice cstr_append_##STYPE##_buf_slice(cstr_##STYPE##_buf_slice buf_slice, TYPE val) \
    {                                                                                                      \
        (*buf_slice.buf)->slice.len = buf_slice.to;                                                        \
        return cstr_append_##STYPE##_buf(buf_slice.buf, val);                                              \
    }

GEN_ALLOC_SLICE_BUF(sslice, , uint8_t)
GEN_APPEND_SLICE_BUF(sslice, , uint8_t)
GEN_ALLOC_SLICE_BUF(const_sslice, const, uint8_t)
GEN_ALLOC_SLICE_BUF(islice, , int)
GEN_APPEND_SLICE_BUF(islice, , int)
GEN_ALLOC_SLICE_BUF(const_islice, const, int)
GEN_ALLOC_SLICE_BUF(uislice, , unsigned int)
GEN_APPEND_SLICE_BUF(uislice, , unsigned int)
GEN_ALLOC_SLICE_BUF(const_uislice, const, unsigned int)

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

#define CSTR_GEN_REV_SLICE(STYPE, BTYPE)  \
    void cstr_rev_##STYPE(cstr_##STYPE s) \
    {                                     \
        long long i = 0, j = s.len - 1;   \
        for (; i < j; i++, j--)           \
        {                                 \
            BTYPE tmp = s.buf[i];         \
            s.buf[i] = s.buf[j];          \
            s.buf[j] = tmp;               \
        }                                 \
    }
CSTR_GEN_REV_SLICE(sslice, uint8_t)
CSTR_GEN_REV_SLICE(islice, int)
CSTR_GEN_REV_SLICE(uislice, unsigned int)

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
