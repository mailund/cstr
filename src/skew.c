#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cstr_internal.h"

// these calculations are only valid for n > 0. For n == 0, they are both
// zero, but there is no need to check for this border case if we never
// have a string with length zero, and we can't because of the sentinel.
static inline unsigned int sa3len(unsigned int n) { return (n - 1) / 3 + 1; }
static inline unsigned int sa12len(unsigned int n) { return n - sa3len(n); }

static unsigned int* get_sa12(unsigned n, unsigned int x[n])
{
    unsigned int* sa12 = malloc(sa12len(n) * sizeof *sa12);
    if (sa12) {
        unsigned int j = 0;
        for (unsigned int i = 0; i < n; i++) {
            if (i % 3 != 0) {
                sa12[j++] = i;
            }
        }
        assert(j == sa12len(n)); // for the static analyser
    }
    return sa12;
}

static unsigned int* get_sa3(unsigned n, unsigned x[n], unsigned sa12[])
{
    unsigned int* sa3 = malloc(sa3len(n) * sizeof *sa3);
    if (sa3) {
        int k = 0;
        // Special case if the last index is in sa3
        if (n % 3 == 1) {
            sa3[k++] = n - 1;
        }
        for (int i = 0; i < sa12len(n); i++) {
            if (sa12[i] % 3 == 1) {
                sa3[k++] = sa12[i] - 1;
            }
        }
        assert(k == sa3len(n)); // for the static analyser
    }
    return sa3;
}

static inline unsigned int safe_idx(unsigned n, unsigned int x[n], unsigned i)
{
    return (i >= n) ? 0 : x[i];
}

static void bucket_sort_with_buffers(unsigned int n, unsigned int x[n],
    unsigned m, unsigned int idx[m],
    unsigned int offset, unsigned int asize,
    unsigned int* buckets,
    unsigned int* buffer)
{
    assert(n > 0 && m > 0 && asize > 0); // helping the static analyser.

    // Compute buckets
    for (int i = 0; i < asize; i++) {
        buckets[i] = 0;
    }
    for (int i = 0; i < m; i++) {
        buckets[safe_idx(n, x, idx[i] + offset)]++;
    }
    for (unsigned int acc = 0, i = 0; i < asize; i++) {
        unsigned int k = buckets[i];
        buckets[i] = acc;
        acc += k;
    }

    // sort
    for (int i = 0; i < m; i++) {
        unsigned int bucket = safe_idx(n, x, idx[i] + offset);
        buffer[buckets[bucket]++] = idx[i];
    }

    // copy sorted back into idx
    memcpy(idx, buffer, m * sizeof(*buffer));
}

static bool bucket_sort(unsigned int n, unsigned int x[n], unsigned int m,
    unsigned int idx[m], unsigned int offset,
    unsigned int asize, errcodes* err)
{
    bool success = false;

    unsigned int* buckets = malloc(asize * sizeof *buckets);
    unsigned int* buffer = malloc(m * sizeof *buffer);
    alloc_error_if(!buckets || !buffer);

    bucket_sort_with_buffers(n, x, m, idx, offset, asize, buckets, buffer);

    success = true;

error:
    free(buckets);
    free(buffer);

    return success;
}

static bool radix3(unsigned int n, unsigned int x[n], unsigned int m,
    unsigned int idx[m], unsigned int asize, errcodes* err)
{
    bool success = false;

    unsigned int* buckets = malloc(asize * sizeof *buckets);
    unsigned int* buffer = malloc(m * sizeof *buffer);
    alloc_error_if(!buckets || !buffer);

    bucket_sort_with_buffers(n, x, m, idx, 2, asize, buckets, buffer);
    bucket_sort_with_buffers(n, x, m, idx, 1, asize, buckets, buffer);
    bucket_sort_with_buffers(n, x, m, idx, 0, asize, buckets, buffer);

    success = true;

error: // even with success, we have to clean...
    free(buckets);
    free(buffer);

    return success;
}

static bool less(unsigned int n, unsigned int x[], unsigned int i,
    unsigned int j, unsigned int isa[])
{
    unsigned int a = safe_idx(n, x, i);
    unsigned int b = safe_idx(n, x, j);
    if (a < b)
        return true;
    if (a > b)
        return false;
    if (i % 3 != 0 && j % 3 != 0)
        return isa[i] < isa[j];
    return less(n, x, i + 1, j + 1, isa);
}

static unsigned int* merge(unsigned int n, unsigned int x[n],
    unsigned int sa12[], unsigned int sa3[])
{

    assert(n > 0); // For the static analyser.
    // We cannot have n==0 because of the sentinel, but the analyser
    // is right that isa could be allocated with length zero, and that
    // would be a potential problem later. It can't happen, though.

    // Allocate isa first. We always have to free it, so we can treat
    // it the same both when we successfully allocate sa and when we dont.
    unsigned int* isa = malloc(n * sizeof *isa);
    if (!isa) {
        return 0;
    }

    unsigned int* sa = malloc(n * sizeof *sa);
    if (!sa) {
        goto done; // isa will be freed there and we will return 0
    }

    // Without a map, the easiest solution for the inverse
    // suffix array is to use an array with the same
    // length as x.
    for (unsigned int i = 0; i < sa12len(n); i++) {
        isa[sa12[i]] = i;
    }

    unsigned int i = 0, j = 0, k = 0;
    while (i < sa12len(n) && j < sa3len(n)) {
        if (less(n, x, sa12[i], sa3[j], isa)) {
            sa[k++] = sa12[i++];
        } else {
            sa[k++] = sa3[j++];
        }
    }
    for (; i < sa12len(n); i++)
        sa[k++] = sa12[i];
    for (; j < sa3len(n); j++)
        sa[k++] = sa3[j];

    assert(k == n); // for the static analyser

done:
    free(isa);

    return sa; // 0 if we had an error, otherwise the merged sa
}

static inline bool equal3(unsigned int n, unsigned int x[n], unsigned int i,
    unsigned int j)
{
    return safe_idx(n, x, i + 0) == safe_idx(n, x, j + 0) && safe_idx(n, x, i + 1) == safe_idx(n, x, j + 1) && safe_idx(n, x, i + 2) == safe_idx(n, x, j + 2);
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

static unsigned int* build_alphabet(unsigned int n, unsigned int x[n],
    unsigned int sa12[],
    unsigned int* new_asize)
{
    // Build the alphabet for u. We build the mapping/encoding
    // of the indices to new letters at the same time.

    assert(sa12len(n) > 0); // helping static analyser
    unsigned int* encoding = malloc(sa12len(n) * sizeof *encoding);
    if (encoding) {
        *new_asize = 1; // start at 1, reserving 0 for sentinel
        encoding[map_x_sa12(sa12[0])] = *new_asize;
        for (int i = 1; i < sa12len(n); i++) {
            if (!equal3(n, x, sa12[i - 1], sa12[i])) {
                (*new_asize)++;
            }
            encoding[map_x_sa12(sa12[i])] = *new_asize;
        }
        // the alphabet is one larger than the largest letter
        (*new_asize)++;
    }
    return encoding;
}

// this u is based on the terminal sentinel always being part of the input, so
// we don't need a central sentinel.
static unsigned int* build_u(unsigned int n, unsigned int encoding[])
{
    unsigned int* u = malloc(sa12len(n) * sizeof *u);
    if (u) {
        int k = 0;
        for (int i = 0; i < sa12len(n); i += 2) {
            u[k++] = encoding[i];
        }
        for (int i = 1; i < sa12len(n); i += 2) {
            u[k++] = encoding[i];
        }
        assert(k == sa12len(n)); // for the static analyser
    }
    return u;
}

static unsigned int* skew_rec(unsigned int n, unsigned int x[n],
    unsigned int asize, errcodes* err)
{
    // set all the pointers to null in case of errors.
    unsigned int *sa = 0, *sa12 = 0, *sa3 = 0, *encoding = 0, *u = 0, *u_sa = 0;

    sa12 = get_sa12(n, x);
    alloc_error_if(!sa12);

    bool ok = radix3(n, x, sa12len(n), sa12, asize, err);
    reraise_error_if(!ok);

    unsigned int new_asize;
    encoding = build_alphabet(n, x, sa12, &new_asize);
    alloc_error_if(!encoding);

    // if the alphabet minus the sentinel matches the length of sa12, then
    // all symbols in sa12 are unique and we do not need to process further.
    if (new_asize - 1 < sa12len(n)) {
        // We need to sort recursively
        u = build_u(n, encoding);
        alloc_error_if(!u);
        free_and_null(encoding); // we might as well clean up early

        u_sa = skew_rec(sa12len(n), u, new_asize, err);
        alloc_error_if(!u_sa);
        free_and_null(u);

        unsigned int m = (sa12len(n) + 1) / 2;
        for (int i = 0; i < sa12len(n); i++) {
            sa12[i] = map_u_x(u_sa[i], m);
        }
        free_and_null(u_sa);
    }

    sa3 = get_sa3(n, x, sa12);
    alloc_error_if(!sa3);

    ok = bucket_sort(n, x, sa3len(n), sa3, 0, asize, err);
    reraise_error_if(!ok);

    sa = merge(n, x, sa12, sa3);
    alloc_error_if(!sa);

    // we free the same memory on success and failure, the only
    // difference in the two exists is whether the error flag is set.
error: // but also success...
    free(sa12);
    free(sa3);
    free(encoding);
    free(u);
    free(u_sa);

    return sa;
}

unsigned int* cstr_skew(alpha* alpha, csslice slice, enum cstr_errcodes* err)
{
    clear_error();

    // make sure allocated pointers are initially null in case of errors
    unsigned int* sa = 0;
    unsigned int* arr = 0;

    // we need to represent up to len+1 (x+sentinel) in
    // unsigned int, so we have an error if we can't.
    size_error_if(slice.len > UINT_MAX - 1);

    arr = cstr_alphabet_map_to_int_new(slice, alpha, err);
    alloc_error_if(!arr);

    sa = skew_rec((unsigned int)slice.len + 1, arr, alpha->size, err);

    // we need to do the same things on error as on return...
error:
    free(arr);

    return sa;
}

unsigned int* cstr_skew_from_string(char const* x, enum cstr_errcodes* err)
{
    clear_error();

    alpha* alpha = 0;
    unsigned int* sa = 0;

    csslice slice = CSTR_CSSLICE_STRING(x);
    // we need to represent up to len+1 (x+sentinel) in
    // unsigned int, so we have an error if we can't.
    size_error_if(slice.len > UINT_MAX - 1);

    alpha = malloc(sizeof *alpha);
    alloc_error_if(!alpha);
    cstr_init_alphabet(alpha, slice);

    sa = cstr_skew(alpha, slice, err);

// we need to do the same things on error as on return...
error: // and also success:
    free(alpha);

    return sa;
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
