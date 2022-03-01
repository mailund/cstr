#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>

#include "unittests.h"

// these calculations are only valid for n > 0. For n == 0, they are both
// zero, but there is no need to check for this border case if we never
// have a string with length zero, and we can't because of the sentinel.
// These operate on size_t, although the suffix arrays can only handle
// unsigned int, but they do that to fit with the slice interface. There
// is a check for the size of input in the public functions.
static inline long long sa3len(long long n) { return (n - 1) / 3 + 1; }
static inline long long sa12len(long long n) { return n - sa3len(n); }

static inline unsigned int safe_idx(cstr_const_uislice x, unsigned int i)
{
    return (i >= x.len) ? 0 : x.buf[i];
}

static void get_sa12(cstr_suffix_array sa12, cstr_const_uislice x)
{
    // For the static analyser...
    assert(sa12.buf && sa12.len > 0);
    assert(sa12.len == sa12len(x.len));

    unsigned int j = 0;
    for (unsigned int i = 0; i < x.len; i++)
    {
        if (i % 3 != 0)
        {
            sa12.buf[j++] = i;
        }
    }

    assert(j == sa12.len); // for the static analyser
}

static void get_sa3(cstr_suffix_array sa3,
                    cstr_suffix_array sa12,
                    cstr_const_uislice x)
{
    // The static analyser is crazy about assertions like this...
    assert(sa3.buf && sa12.buf && sa3.len > 0 && sa3.len == sa3len(x.len));

    int k = 0;
    // Special case if the last index is in sa3
    if (x.len % 3 == 1)
    {
        sa3.buf[k++] = (unsigned int)x.len - 1;
    }
    for (int i = 0; i < sa12.len; i++)
    {
        if (sa12.buf[i] % 3 == 1)
        {
            sa3.buf[k++] = sa12.buf[i] - 1;
        }
    }
    assert(k == sa3.len); // for the static analyser
}

static void bucket_sort_with_buffers(cstr_const_uislice x,
                                     cstr_uislice idx,
                                     unsigned int offset,
                                     unsigned int asize,
                                     unsigned int *restrict buckets,
                                     unsigned int *restrict buffer)
{
    // helping the static analyser.
    assert(x.len > 0 && idx.len > 0 && asize > 0);

    // Compute buckets
    for (unsigned int i = 0; i < asize; i++)
    {
        buckets[i] = 0;
    }
    for (unsigned int i = 0; i < idx.len; i++)
    {
        buckets[safe_idx(x, idx.buf[i] + offset)]++;
    }
    for (unsigned int acc = 0, i = 0; i < asize; i++)
    {
        unsigned int k = buckets[i];
        buckets[i] = acc;
        acc += k;
    }

    // sort
    for (int i = 0; i < idx.len; i++)
    {
        unsigned int bucket = safe_idx(x, idx.buf[i] + offset);
        buffer[buckets[bucket]++] = idx.buf[i];
    }

    // copy sorted back into idx
    memcpy(idx.buf, buffer, (size_t)idx.len * sizeof(*buffer));
}

static void bucket_sort(cstr_const_uislice x,
                        cstr_uislice idx,
                        unsigned int offset,
                        unsigned int asize)
{
    unsigned int *buckets = cstr_malloc((size_t)asize * sizeof *buckets);
    unsigned int *buffer = cstr_malloc((size_t)idx.len * sizeof *buffer);
    bucket_sort_with_buffers(x, idx, offset, asize, buckets, buffer);

    free(buckets);
    free(buffer);
}

static void radix3(cstr_const_uislice x, cstr_uislice idx, unsigned int asize)
{
    unsigned int *buckets = cstr_malloc((size_t)asize * sizeof *buckets);
    unsigned int *buffer = cstr_malloc((size_t)idx.len * sizeof *buffer);

    bucket_sort_with_buffers(x, idx, 2, asize, buckets, buffer);
    bucket_sort_with_buffers(x, idx, 1, asize, buckets, buffer);
    bucket_sort_with_buffers(x, idx, 0, asize, buckets, buffer);

    free(buckets);
    free(buffer);
}

static bool less(cstr_const_uislice x,
                 unsigned int i, unsigned int j,
                 unsigned int isa[])
{
    unsigned int a = safe_idx(x, i);
    unsigned int b = safe_idx(x, j);

    if (a < b)
        return true;
    if (a > b)
        return false;
    if (i % 3 != 0 && j % 3 != 0)
        return isa[i] < isa[j];

    return less(x, i + 1, j + 1, isa);
}

static void merge(cstr_suffix_array sa,
                  cstr_const_uislice x,
                  cstr_suffix_array sa12,
                  cstr_suffix_array sa3)
{
    // For the static analyser.
    // We cannot have n==0 because of the sentinel, but the analyser
    // is right that isa could be allocated with length zero, and that
    // would be a potential problem later. It can't happen, though.
    assert(x.len > 0);
    assert(sa.buf && x.buf && sa12.buf && sa3.buf);

    unsigned int *isa = cstr_malloc((size_t)x.len * sizeof *isa);

    // Without a map, the easiest solution for the inverse
    // suffix array is to use an array with the same
    // length as x.
    for (unsigned int i = 0; i < sa12.len; i++)
    {
        isa[sa12.buf[i]] = i;
    }

    unsigned int i = 0, j = 0, k = 0;
    while (i < sa12.len && j < sa3.len)
    {
        if (less(x, sa12.buf[i], sa3.buf[j], isa))
        {
            sa.buf[k++] = sa12.buf[i++];
        }
        else
        {
            sa.buf[k++] = sa3.buf[j++];
        }
    }
    for (; i < sa12.len; i++)
        sa.buf[k++] = sa12.buf[i];
    for (; j < sa3.len; j++)
        sa.buf[k++] = sa3.buf[j];

    assert(k == x.len); // for the static analyser

    free(isa);
}

static inline bool equal3(cstr_const_uislice x,
                          unsigned int i, unsigned int j)
{
    return safe_idx(x, i + 0) == safe_idx(x, j + 0) &&
           safe_idx(x, i + 1) == safe_idx(x, j + 1) &&
           safe_idx(x, i + 2) == safe_idx(x, j + 2);
}

// Map from indices in x to indices in sa12
static inline unsigned int map_x_sa12(unsigned int k)
{
    return 2 * (k / 3) + (k % 3) - 1;
}
// Map from indices in u to indices in x
static inline unsigned int map_u_x(unsigned int i, unsigned int m)
{
    return (i < m) ? (1 + 3 * i) : (2 + 3 * (i - m));
}

static unsigned int build_alphabet(unsigned int encoding[],
                                   cstr_const_uislice x,
                                   cstr_suffix_array sa12)
{
    // Build the alphabet for u. We build the mapping/encoding
    // of the indices to new letters at the same time.
    assert(sa12.buf && sa12.len > 0); // helping static analyser

    unsigned int new_asize = 1; // start at 1, reserving 0 for sentinel
    encoding[map_x_sa12(sa12.buf[0])] = new_asize;

    for (int i = 1; i < sa12.len; i++)
    {
        if (!equal3(x, sa12.buf[i - 1], sa12.buf[i]))
        {
            new_asize++;
        }
        encoding[map_x_sa12(sa12.buf[i])] = new_asize;
    }

    // the alphabet is one larger than the largest letter
    new_asize++;

    return new_asize;
}

// this u is based on the terminal sentinel always being part of the input, so
// we don't need a central sentinel.
static void build_u(cstr_uislice u, unsigned int const encoding[])
{
    assert(u.buf);

    unsigned int k = 0;
    for (unsigned int i = 0; i < u.len; i += 2)
    {
        u.buf[k++] = encoding[i];
    }
    for (unsigned int i = 1; i < u.len; i += 2)
    {
        u.buf[k++] = encoding[i];
    }
    assert(k == u.len); // for the static analyser
}

static void skew_rec(cstr_suffix_array sa, cstr_const_uislice x, unsigned int asize)
{
    cstr_suffix_array *sa12 = cstr_alloc_uislice(sa12len(x.len));
    get_sa12(*sa12, x);
    radix3(x, *sa12, asize);

    unsigned int *encoding = cstr_malloc((size_t)sa12->len * sizeof *encoding);
    unsigned int new_asize = build_alphabet(encoding, x, *sa12);

    // if the alphabet minus the sentinel matches the length of sa12, then
    // all symbols in sa12 are unique and we do not need to process further.
    if (new_asize - 1 < sa12->len)
    {
        // We need to sort recursively
        cstr_uislice *u = cstr_alloc_uislice(sa12->len);
        build_u(*u, encoding);
        CSTR_FREE_NULL(encoding);

        cstr_suffix_array *u_sa = cstr_alloc_uislice(u->len);

        skew_rec(*u_sa, CSTR_SLICE_CONST_CAST(*u), new_asize);

        unsigned int m = (unsigned int)(u_sa->len + 1) / 2;
        for (unsigned int i = 0; i < u_sa->len; i++)
        {
            sa12->buf[i] = map_u_x(u_sa->buf[i], m);
        }

        free(u);
        free(u_sa);
    }

    cstr_suffix_array *sa3 = cstr_alloc_uislice(sa3len(x.len));
    get_sa3(*sa3, *sa12, x);

    bucket_sort(x, *sa3, /* offset */ 0, asize);

    merge(sa, x, *sa12, *sa3);

    free(sa12);
    free(sa3);
    free(encoding);
}

void cstr_skew(cstr_suffix_array sa, cstr_const_uislice x, cstr_alphabet *alpha)
{
    // we need to store indices in int, so there is a limit to the
    // length.
    assert(x.len <= INT_MAX - 1);
    skew_rec(sa, x, (unsigned int)alpha->size);
}

#ifdef GEN_UNIT_TESTS // unit testing of static functions...

TL_TEST(skew_test_len_calc)
{
    TL_BEGIN();

    // with length 1 we have one n3 and zero n12.
    unsigned int n12 = 0, n3 = 0;

    // we track the last index, so the length is one more.
    // this makes it easier to count, because last index is something
    // we have seen and counted.
    for (unsigned int last_index = 0; last_index <= 100; last_index++)
    {
        if (last_index % 3 == 0)
        {
            n3++;
        }
        else
        {
            n12++;
        }
        unsigned int n = last_index + 1;
        TL_ERROR_IF(n12 != sa12len(n));
        TL_ERROR_IF(n3 != sa3len(n));
    }

    TL_END();
}

#endif
