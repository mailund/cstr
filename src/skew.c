#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cstr_internal.h"

// these calculations are only valid for n > 0. For n == 0, they are both
// zero, but there is no need to check for this border case if we never
// have a string with length zero, and we can't because of the sentinel.
// These operate on size_t, although the suffix arrays cann only handle
// unsigned int, but they do that to fit with the slice interface. There
// is a check for the size of input in the public functions.
static inline size_t sa3len(size_t n) { return (n - 1) / 3 + 1; }
static inline size_t sa12len(size_t n) { return n - sa3len(n); }

static inline unsigned int safe_idx(islice x, unsigned i)
{
    return (i >= x.len) ? 0 : x.buf[i];
}

static void get_sa12(islice sa12, islice x)
{
    assert(sa12.len > 0 && sa12.len == sa12len(x.len)); // For the static analyser...
    unsigned int j = 0;
    for (unsigned int i = 0; i < x.len; i++) {
        if (i % 3 != 0) {
            sa12.buf[j++] = i;
        }
    }
    assert(j == sa12.len); // for the static analyser
}

static void get_sa3(islice sa3, islice x, islice sa12)
{
    assert(sa3.len > 0 && sa3.len == sa3len(x.len));
    int k = 0;
    // Special case if the last index is in sa3
    if (x.len % 3 == 1) {
        sa3.buf[k++] = (unsigned int)x.len - 1;
    }
    for (int i = 0; i < sa12.len; i++) {
        if (sa12.buf[i] % 3 == 1) {
            sa3.buf[k++] = sa12.buf[i] - 1;
        }
    }
    assert(k == sa3.len); // for the static analyser
}

static void bucket_sort_with_buffers(
    islice x,
    islice idx,
    unsigned int offset,
    unsigned int asize,
    unsigned int* buckets,
    unsigned int* buffer)
{
    assert(x.len > 0 && idx.len > 0 && asize > 0); // helping the static analyser.

    // Compute buckets
    for (unsigned int i = 0; i < asize; i++) {
        buckets[i] = 0;
    }
    for (unsigned int i = 0; i < idx.len; i++) {
        buckets[safe_idx(x, idx.buf[i] + offset)]++;
    }
    for (unsigned int acc = 0, i = 0; i < asize; i++) {
        unsigned int k = buckets[i];
        buckets[i] = acc;
        acc += k;
    }

    // sort
    for (unsigned int i = 0; i < idx.len; i++) {
        unsigned int bucket = safe_idx(x, idx.buf[i] + offset);
        buffer[buckets[bucket]++] = idx.buf[i];
    }

    // copy sorted back into idx
    memcpy(idx.buf, buffer, idx.len * sizeof(*buffer));
}

static bool bucket_sort(
    islice x,
    islice idx,
    unsigned int offset,
    unsigned int asize, errcodes* err)
{
    bool success = false;

    unsigned int* buckets = malloc(asize * sizeof *buckets);
    unsigned int* buffer = malloc(idx.len * sizeof *buffer);
    alloc_error_if(!buckets || !buffer);

    bucket_sort_with_buffers(x, idx, offset, asize, buckets, buffer);

    success = true;

error:
    free(buckets);
    free(buffer);

    return success;
}

static bool radix3(
    islice x,
    islice idx,
    unsigned int asize,
    errcodes* err)
{
    bool success = false;

    unsigned int* buckets = malloc(asize * sizeof *buckets);
    unsigned int* buffer = malloc(idx.len * sizeof *buffer);
    alloc_error_if(!buckets || !buffer);

    bucket_sort_with_buffers(x, idx, 2, asize, buckets, buffer);
    bucket_sort_with_buffers(x, idx, 1, asize, buckets, buffer);
    bucket_sort_with_buffers(x, idx, 0, asize, buckets, buffer);

    success = true;

error: // even with success, we have to clean...
    free(buckets);
    free(buffer);

    return success;
}

static bool less(
    islice x,
    unsigned int i,
    unsigned int j,
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

static bool merge(
    islice sa,
    islice x,
    islice sa12,
    islice sa3,
    enum cstr_errcodes* err)
{
    bool ok = false;

    assert(x.len > 0); // For the static analyser.
    // We cannot have n==0 because of the sentinel, but the analyser
    // is right that isa could be allocated with length zero, and that
    // would be a potential problem later. It can't happen, though.

    unsigned int* isa = malloc(x.len * sizeof *isa);
    alloc_error_if(!isa);

    // Without a map, the easiest solution for the inverse
    // suffix array is to use an array with the same
    // length as x.
    for (unsigned int i = 0; i < sa12.len; i++) {
        isa[sa12.buf[i]] = i;
    }

    unsigned int i = 0, j = 0, k = 0;
    while (i < sa12.len && j < sa3.len) {
        if (less(x, sa12.buf[i], sa3.buf[j], isa)) {
            sa.buf[k++] = sa12.buf[i++];
        } else {
            sa.buf[k++] = sa3.buf[j++];
        }
    }
    for (; i < sa12.len; i++)
        sa.buf[k++] = sa12.buf[i];
    for (; j < sa3.len; j++)
        sa.buf[k++] = sa3.buf[j];

    assert(k == x.len); // for the static analyser
    ok = true; // successfull completion

    // both error and success ends up here, but ok
    // tells us which it is.
error:
    free(isa);
    return ok;
}

static inline bool equal3(
    islice x,
    unsigned int i,
    unsigned int j)
{
    return safe_idx(x, i + 0) == safe_idx(x, j + 0)
        && safe_idx(x, i + 1) == safe_idx(x, j + 1)
        && safe_idx(x, i + 2) == safe_idx(x, j + 2);
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

static unsigned int* build_alphabet(
    islice x,
    islice sa12,
    unsigned int* new_asize)
{
    // Build the alphabet for u. We build the mapping/encoding
    // of the indices to new letters at the same time.

    assert(sa12.len > 0); // helping static analyser
    unsigned int* encoding = malloc(sa12.len * sizeof *encoding);
    if (encoding) {
        *new_asize = 1; // start at 1, reserving 0 for sentinel
        encoding[map_x_sa12(sa12.buf[0])] = *new_asize;
        for (int i = 1; i < sa12.len; i++) {
            if (!equal3(x, sa12.buf[i - 1], sa12.buf[i])) {
                (*new_asize)++;
            }
            encoding[map_x_sa12(sa12.buf[i])] = *new_asize;
        }
        // the alphabet is one larger than the largest letter
        (*new_asize)++;
    }
    return encoding;
}

// this u is based on the terminal sentinel always being part of the input, so
// we don't need a central sentinel.
static void build_u(
    islice u,
    unsigned int encoding[])
{
    int k = 0;
    for (unsigned int i = 0; i < u.len; i += 2) {
        u.buf[k++] = encoding[i];
    }
    for (unsigned int i = 1; i < u.len; i += 2) {
        u.buf[k++] = encoding[i];
    }
    assert(k == u.len); // for the static analyser
}

static bool skew_rec(
    islice sa,
    islice x,
    unsigned int asize,
    errcodes* err)
{
    bool func_ok = false, ok = false; // first for the return value, second for testing along the way
    islice sa12 = CSTR_NIL_SLICE, sa3 = CSTR_NIL_SLICE, u = CSTR_NIL_SLICE, u_sa = CSTR_NIL_SLICE;
    unsigned int *encoding = 0;

    ok = cstr_alloc_islice_buffer(&sa12, sa12len(x.len));
    alloc_error_if(!ok);
    get_sa12(sa12, x);

    ok = radix3(x, sa12, asize, err);
    reraise_error_if(!ok);

    unsigned int new_asize;
    encoding = build_alphabet(x, sa12, &new_asize);
    alloc_error_if(!encoding);

    // if the alphabet minus the sentinel matches the length of sa12, then
    // all symbols in sa12 are unique and we do not need to process further.
    if (new_asize - 1 < sa12len(x.len)) {
        // We need to sort recursively
        ok = cstr_alloc_islice_buffer(&u, sa12len(x.len));
        alloc_error_if(!ok);

        build_u(u, encoding);
        free_and_null(encoding); // we might as well clean up early

        // get memory for the recursive suffix array
        ok = cstr_alloc_islice_buffer(&u_sa, u.len);
        alloc_error_if(!ok);
        
        ok = skew_rec(u_sa, u, new_asize, err);
        alloc_error_if(!ok);
        cstr_free_islice_buffer(&u); // done with u now...
        
        unsigned int m = (unsigned int)(u.len + 1) / 2;
        for (unsigned int i = 0; i < u.len; i++) {
            sa12.buf[i] = map_u_x(u_sa.buf[i], m);
        }
        cstr_free_islice_buffer(&u_sa); // done with u_sa.
    }

    ok = cstr_alloc_islice_buffer(&sa3, sa3len(x.len));
    alloc_error_if(!ok);
    get_sa3(sa3, x, sa12);

    ok = bucket_sort(x, sa3, 0, asize, err);
    reraise_error_if(!ok);

    func_ok = merge(sa, x, sa12, sa3, err);
    
    // ok is now true, so we will return success after cleanup.

    // we free the same memory on success and failure, the only
    // difference in the two exists is the value of 'ok'.
error: // but also success...
    cstr_free_islice_buffer(&sa12);
    cstr_free_islice_buffer(&sa3);
    cstr_free_islice_buffer(&u);
    cstr_free_islice_buffer(&u_sa);
    free(encoding);
    
    return func_ok;
}

bool cstr_skew(
    islice sa,
    csslice x,
    alpha* alpha,
    enum cstr_errcodes* err)
{
    unsigned int* mapped_x = 0;
    bool ok = false;

    clear_error();
    // we need to store indices in unsigned int, so there is a limit to the length.
    size_error_if(x.len > UINT_MAX - 1);

    mapped_x = cstr_alphabet_map_to_int_new(x, alpha, err);
    alloc_error_if(!mapped_x);

    ok = skew_rec(sa, CSTR_ISLICE(mapped_x, x.len + 1), alpha->size, err);
    // now ok has the final value, and we return it after cleanup.

    // we need to do the same things on error as on return...
error:
    free(mapped_x);

    return ok;
}

unsigned int* cstr_skew_new(
    csslice x,
    alpha* alpha,
    enum cstr_errcodes* err)
{
    unsigned int* sa = 0;

    clear_error();

    // we need to store indices in unsigned int, so there is a limit to the length
    size_error_if(x.len > UINT_MAX - 1);

    sa = malloc((x.len + 1) * sizeof(*sa));
    alloc_error_if(!sa);

    bool ok = cstr_skew(CSTR_ISLICE(sa, x.len + 1), x, alpha, err);
    reraise_error_if(!ok);

    return sa; // success

error:
    free(sa);
    return 0;
}

#if GEN_UNIT_TESTS // unit testing of static functions...

TL_TEST(skew_test_len_calc)
{
    TL_BEGIN();

    // with length 1 we have one n3 and zero n12.
    unsigned int n12 = 0, n3 = 0;

    // we track the last index, so the length is one more.
    // this makes it easier to count, because last index is something
    // we have seen and counted.
    for (unsigned int last_index = 0; last_index <= 100; last_index++) {
        if (last_index % 3 == 0) {
            n3++;
        } else {
            n12++;
        }
        unsigned int n = last_index + 1;
        TL_ERROR_IF(n12 != sa12len(n));
        TL_ERROR_IF(n3 != sa3len(n));
    }

    TL_END();
}

#endif
