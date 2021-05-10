#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "misc.h"

static inline
int sa3len(int n)
{
    return (n-1)/3 + 1;
}

static inline
int sa12len(int n)
{
    return n - sa3len(n);
}

static
int * get_sa12(int n, int x[n])
{
    int *sa12 = malloc(sa12len(n) * sizeof *sa12);
    for (int i = 0, j = 0; i < n; i++) {
        if (i % 3 != 0) {
            sa12[j++] = i;
        }
    }
    return sa12;
}

static
int * get_sa3(int n, int x[n], int sa12[])
{
    int *sa3 = malloc(sa3len(n) * sizeof *sa3);
    int k = 0;
    // Special case if the last index is in sa3
    if (n % 3 == 1) {
        sa3[k++] = n - 1;
    }
    int m = sa12len(n);
    for (int i = 0; i < m; i++) {
        if (sa12[i] % 3 == 1) {
            sa3[k++] = sa12[i] - 1;
        }
    }
    return sa3;
}

static inline
int safe_idx(int n, int x[n], int i)
{
    return (i >= n) ? 0 : x[i];
}

static
void bucket_sort(int n, int x[n],
                 int m, int idx[m],
                 int offset, int asize)
{
    int *buckets = malloc(asize * sizeof *buckets);
    int *sorted = malloc(m * sizeof *sorted);
    
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
        sorted[buckets[bucket]++] = idx[i];
    }

    // copy sorted back into idx
    for (int i = 0; i < m; i++) {
        idx[i] = sorted[i];
    }

    free(buckets);
    free(sorted);
}

static
void radix3(int n, int x[n],
            int m, int idx[m],
            int asize)
{
    // This would be more efficient if we reused
    // the buffers in bucket_sort(), but we are
    // keeping the code simple...
    bucket_sort(n, x, m, idx, 2, asize);
    bucket_sort(n, x, m, idx, 1, asize);
    bucket_sort(n, x, m, idx, 0, asize);
}

static
bool less(int n, int x[], int i, int j, int isa[])
{
    int a = safe_idx(n, x, i);
    int b = safe_idx(n, x, j);
    if (a < b) return true;
    if (a > b) return false;
    if (i%3 != 0 && j%3 != 0) return isa[i] < isa[j];
    return less(n, x, i + 1, j + 1, isa);
}

static
int * merge(int n, int x[n], int sa12[], int sa3[])
{
    int * sa = malloc(n * sizeof *sa);
    int n12 = sa12len(n);
    int n3 = sa3len(n);

    // Without a map, the easiest solution for the inverse
    // suffix array is to use an array with the same
    // length as x.
    int *isa = malloc(n * sizeof *isa);
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
    for (; i < n12; i++) sa[k++] = sa12[i];
    for (; j < n3; j++)  sa[k++] = sa3[j];

    free(isa);
    return sa;
}

static inline
bool equal3(int n, int x[n], int i, int j)
{
    return safe_idx(n, x, i + 0) == safe_idx(n, x, j + 0) &&
           safe_idx(n, x, i + 1) == safe_idx(n, x, j + 1) &&
           safe_idx(n, x, i + 2) == safe_idx(n, x, j + 2);
}

// Map from indices in x to indices in sa12
static inline int map_x_sa12(int k) 
{
    return 2 * (k / 3) + (k % 3) - 1;
}
// Map from indices in u to indices in x
static inline int map_u_x(int i, int m)
{
    if (i < m) return 1 + 3 * i;
    else return 2 + 3 * (i - m - 1);
}

static
int * build_alphabet(int n, int x[n], int sa12[], int *new_asize)
{
    // Build the alphabet for u. We build the mapping/encoding
    // of the indices to new letters at the same time.
    *new_asize = 2;
    int *encoding = malloc(sa12len(n) * sizeof *encoding);
    encoding[map_x_sa12(sa12[0])] = 2;
    for (int i = 1; i < sa12len(n); i++) {
        if (!equal3(n, x, sa12[i - 1], sa12[i])) {
            (*new_asize)++;
        }
        encoding[map_x_sa12(sa12[i])] = *new_asize;
    }
    // the alphabet is one larger than the largest letter
    (*new_asize)++;
    return encoding;
}

static
int *build_u(int n, int encoding[])
{
    int *u = malloc((sa12len(n) + 1) * sizeof *u);
    int k = 0;
    for (int i = 0; i < sa12len(n); i += 2) {
        u[k++] = encoding[i];
    }
    u[k++] = 1;
    for (int i = 1; i < sa12len(n); i += 2) {
        u[k++] = encoding[i];
    }
    return u;
}

static
int * skew_rec(int n, int x[], int asize)
{
    int *sa12 = get_sa12(n, x);
    radix3(n, x, sa12len(n), sa12, asize);

    int new_asize;
    int *encoding = build_alphabet(n, x, sa12, &new_asize);
    
    if (new_asize - 2 < sa12len(n)) {
        // We need to sort recursively
        int *u = build_u(n, encoding);
        // we don't need encoding any longer, so free it.
        // set it to NULL to avoid problems with freeing it below.
        free(encoding); encoding = 0;
        
        int *usa = skew_rec(sa12len(n) + 1, u, new_asize);
        int m = (sa12len(n) + 1) / 2;
        assert(usa[0] == m); // mid sentinel is first
        for (int i = 0, k = 0; i < sa12len(n) + 1; i++) {
            if (usa[i] == m) continue;
            sa12[k++] = map_u_x(usa[i], m);
        }

        free(usa);
        free(u);
    }

    free(encoding);
    
    int *sa3 = get_sa3(n, x, sa12);
    bucket_sort(n, x, sa3len(n), sa3, 0, asize);

    int *sa = merge(n, x, sa12, sa3);
    free(sa12); free(sa3);
    return sa;
}

int * skew(char const *x)
{
    int n = strlen(x);
    int *ix = string_to_int(x, n);
    int *sa = skew_rec(n, ix, 256);
    free(ix);
    return sa;
}

