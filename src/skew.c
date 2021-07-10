#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cstr_internal.h"

static inline int sa3len(int n) { return (n - 1) / 3 + 1; }
static inline int sa12len(int n) { return n - sa3len(n); }

static int *get_sa12(int n, int x[n]) {
    int *sa12 = malloc(sa12len(n) * sizeof *sa12);
    if (sa12) {
        for (int i = 0, j = 0; i < n; i++) {
            if (i % 3 != 0) {
                sa12[j++] = i;
            }
        }
    }
    return sa12;
}

static int *get_sa3(int n, int x[n], int sa12[]) {
    int *sa3 = malloc(sa3len(n) * sizeof *sa3);
    if (sa3) {
        int k = 0;
        // Special case if the last index is in sa3
        if (n % 3 == 1) {
            sa3[k++] = n - 1;
        }
        int m = sa12len(n);
        assert(m > 0);
        for (int i = 0; i < m; i++) {
            if (sa12[i] % 3 == 1) {
                sa3[k++] = sa12[i] - 1;
            }
        }
    }
    return sa3;
}

static inline int safe_idx(int n, int x[n], int i) {
    return (i >= n) ? 0 : x[i];
}

static void bucket_sort_with_buffers(int n, int x[n], int m, int idx[m],
                                     int offset, int asize, int *buckets,
                                     int *buffer) {
    assert(n > 0 && m > 0 && asize > 0); // helping the static analyser.

    // Compute buckets
    for (int i = 0; i < asize; i++) {
        buckets[i] = 0;
    }
    for (int i = 0; i < m; i++) {
        buckets[safe_idx(n, x, idx[i] + offset)]++;
    }
    for (int acc = 0, i = 0; i < asize; i++) {
        int k = buckets[i];
        buckets[i] = acc;
        acc += k;
    }

    // sort
    for (int i = 0; i < m; i++) {
        int bucket = safe_idx(n, x, idx[i] + offset);
        buffer[buckets[bucket]++] = idx[i];
    }

    // copy sorted back into idx
    memcpy(idx, buffer, m * sizeof(*buffer));
}

static void bucket_sort(int n, int x[n], int m, int idx[m], int offset,
                        int asize, errcodes *err) {
    int *buckets = 0, *buffer = 0;

    buckets = malloc(asize * sizeof *buckets);
    alloc_error_if(!buckets);

    buffer = malloc(m * sizeof *buffer);
    alloc_error_if(!buffer);

    bucket_sort_with_buffers(n, x, m, idx, offset, asize, buckets, buffer);

success:
error:
    free(buckets);
    free(buffer);
}

static void radix3(int n, int x[n], int m, int idx[m], int asize,
                   errcodes *err) {
    int *buckets = 0, *buffer = 0;

    buckets = malloc(asize * sizeof *buckets);
    alloc_error_if(!buckets);

    buffer = malloc(m * sizeof *buffer);
    alloc_error_if(!buffer);

    bucket_sort_with_buffers(n, x, m, idx, 2, asize, buckets, buffer);
    bucket_sort_with_buffers(n, x, m, idx, 1, asize, buckets, buffer);
    bucket_sort_with_buffers(n, x, m, idx, 0, asize, buckets, buffer);

success:
error:
    free(buckets);
    free(buffer);
}

static bool less(int n, int x[], int i, int j, int isa[]) {
    int a = safe_idx(n, x, i);
    int b = safe_idx(n, x, j);
    if (a < b)
        return true;
    if (a > b)
        return false;
    if (i % 3 != 0 && j % 3 != 0)
        return isa[i] < isa[j];
    return less(n, x, i + 1, j + 1, isa);
}

static int *merge(int n, int x[n], int sa12[], int sa3[]) {
    // Allocate isa first. We always have to free it, so we can treat
    // it the same both when we successfully allocate sa and when we dont.
    int *isa = malloc(n * sizeof *isa);
    if (!isa)
        return 0;

    int *sa = malloc(n * sizeof *sa);
    if (!sa)
        goto done; // isa will be freed there and we will return 0

    int n12 = sa12len(n);
    int n3 = sa3len(n);

    // Without a map, the easiest solution for the inverse
    // suffix array is to use an array with the same
    // length as x.
    for (int i = 0; i < sa12len(n); i++) {
        isa[sa12[i]] = i;
    }

    int i = 0, j = 0, k = 0;
    while (i < n12 && j < n3) {
        if (less(n, x, sa12[i], sa3[j], isa)) {
            sa[k++] = sa12[i++];
        } else {
            sa[k++] = sa3[j++];
        }
    }
    for (; i < n12; i++)
        sa[k++] = sa12[i];
    for (; j < n3; j++)
        sa[k++] = sa3[j];

done:
    free(isa);

    return sa; // 0 if we had an error, otherwise the merged sa
}

static inline bool equal3(int n, int x[n], int i, int j) {
    return safe_idx(n, x, i + 0) == safe_idx(n, x, j + 0) &&
           safe_idx(n, x, i + 1) == safe_idx(n, x, j + 1) &&
           safe_idx(n, x, i + 2) == safe_idx(n, x, j + 2);
}

// Map from indices in x to indices in sa12
static inline int map_x_sa12(int k) { return 2 * (k / 3) + (k % 3) - 1; }
// Map from indices in u to indices in x
static inline int map_u_x(int i, int m) {
    if (i < m)
        return 1 + 3 * i;
    else
        return 2 + 3 * (i - m - 1);
}

static int *build_alphabet(int n, int x[n], int sa12[], int *new_asize) {
    // Build the alphabet for u. We build the mapping/encoding
    // of the indices to new letters at the same time.

    assert(sa12len(n) > 0); // helping static analyser
    int *encoding = malloc(sa12len(n) * sizeof *encoding);
    if (encoding) {
        *new_asize = 2;
        encoding[map_x_sa12(sa12[0])] = 2;
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

static int *build_u(int n, int encoding[]) {
    int *u = malloc((sa12len(n) + 1) * sizeof *u);
    if (u) {
        int k = 0;
        for (int i = 0; i < sa12len(n); i += 2) {
            u[k++] = encoding[i];
        }
        u[k++] = 1;
        for (int i = 1; i < sa12len(n); i += 2) {
            u[k++] = encoding[i];
        }
    }
    return u;
}

static int *skew_rec(int n, int x[], int asize, errcodes *err) {
    // make sure that sa is initially null, so we return null
    // if there are errors along the way.
    int *sa = 0;
    
    // Set all pointers to null up front, so it is safe
    // to free them if we have to bail. It costs a little in
    // runtime, but makes error handling a lot easier.
    int *sa12 = 0, *sa3 = 0, *encoding = 0, *u = 0, *u_sa = 0;

    sa12 = get_sa12(n, x);
    alloc_error_if(!sa12);

    radix3(n, x, sa12len(n), sa12, asize, err);
    error_if(*err != CSTR_NO_ERROR, *err);

    int new_asize;
    encoding = build_alphabet(n, x, sa12, &new_asize);
    alloc_error_if(!encoding);

    if (new_asize - 2 < sa12len(n)) {
        // We need to sort recursively
        u = build_u(n, encoding);
        alloc_error_if(!u);

        u_sa = skew_rec(sa12len(n) + 1, u, new_asize, err);
        alloc_error_if(!u_sa);

        int m = (sa12len(n) + 1) / 2;
        assert(u_sa[0] == m); // mid sentinel is first
        for (int i = 0, k = 0; i < sa12len(n) + 1; i++) {
            if (u_sa[i] == m)
                continue;
            sa12[k++] = map_u_x(u_sa[i], m);
        }
    }

    sa3 = get_sa3(n, x, sa12);
    alloc_error_if(!sa3);

    bucket_sort(n, x, sa3len(n), sa3, 0, asize, err);
    error_if(*err != CSTR_NO_ERROR, *err);

    sa = merge(n, x, sa12, sa3);
    alloc_error_if(!sa);

    // we free the same memory on success and failure, the only
    // difference in the two exists is whether the error flag is set.
error:
success:
    free(sa12);
    free(sa3);
    free(encoding);
    free(u);
    free(u_sa);

    return sa;
}

int *cstr_skew(struct cstr_alphabet *alpha, struct cstr_str_slice slice,
               enum cstr_errcodes *err) {
    int *sa = 0; // make sure sa is initially null in case of errors
    
    clear_error();

    int *arr = cstr_alphabet_map_to_int(alpha, slice, err);
    alloc_error_if(!arr);

    sa = skew_rec(slice.len + 1, arr, alpha->size, err);

    // we need to do the same things on error as on return...
error:
success:
    free(arr);

    return sa;
}

int *cstr_skew_from_string(char const *x, enum cstr_errcodes *err) {
    int *sa = 0; // make sure sa is initially null in case of errors
    
    clear_error();

    struct cstr_str_slice slice = cstr_slice_from_string((char *)x);

    // declare these as null pointers up front, so we can
    // safely handle them in error handling...
    struct cstr_alphabet *alpha = cstr_alphabet_from_slice(slice, err);
    alloc_error_if(!alpha);

    sa = cstr_skew(alpha, slice, err);

    // we need to do the same things on error as on return...
error:
success:
    free(alpha);

    return sa;
}
