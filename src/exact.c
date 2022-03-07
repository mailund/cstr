#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "cstr.h"

typedef long long (*exact_next_fn)(cstr_exact_matcher *);
typedef void (*exact_free_fn)(cstr_exact_matcher *);

#define SHARED                                        \
    cstr_exact_matcher matcher; /* MUST come first */ \
    cstr_const_sslice x;                              \
    cstr_const_sslice p;

// Initialising shared bits
#define MATCHER(VTAB, X, P)       \
    .matcher = {.vtab = &(VTAB)}, \
    .x = (X),                     \
    .p = (P)

// Helper macro for initialising matcher v-tables
#define MATCHER_VTAB(NEXT, FREE)   \
    .next = (exact_next_fn)(NEXT), \
    .free = (exact_free_fn)(FREE),

// macros for readability
#define x(S) ((S)->x.buf)
#define p(S) ((S)->p.buf)
#define n(S) ((S)->x.len)
#define m(S) ((S)->p.len)

// Naive O(nm) algorithm
struct naive_matcher_state
{
    SHARED
    long long i;
};

static long long naive_next(struct naive_matcher_state *s)
{
    for (; s->i < n(s); s->i++)
    {
        for (long long j = 0; j < m(s); j++)
        {
            if (x(s)[s->i + j] != p(s)[j])
            {
                break;
            }
            if (j == m(s) - 1)
            {
                // a match
                s->i++; // next time, start from here
                return s->i - 1;
            }
        }
    }
    return -1; // If we get here, we are done.
}

static cstr_exact_matcher_vtab naive_vtab = {MATCHER_VTAB(naive_next, free)};
cstr_exact_matcher *cstr_naive_matcher(cstr_const_sslice x, cstr_const_sslice p)
{
    struct naive_matcher_state *state = cstr_malloc(sizeof *state);
    *state = (struct naive_matcher_state){
        MATCHER(naive_vtab, x, p), .i = 0};
    return (cstr_exact_matcher *)state;
}

// Border array algorithm O(n+m)

static void compute_border_array(cstr_const_sslice p, long long *ba)
{
    // Border array
    ba[0] = 0;
    for (long long i = 1; i < p.len; ++i)
    {
        long long b = ba[i - 1];
        while (b > 0 && p.buf[i] != p.buf[b])
        {
            b = ba[b - 1];
        }
        ba[i] = (p.buf[i] == p.buf[b]) ? b + 1 : 0;
    }

    // restricted border array
    for (long long i = 0; i < p.len - 1; i++)
    {
        if (ba[i] > 0 && p.buf[ba[i]] == p.buf[i + 1])
        {
            ba[i] = ba[ba[i] - 1];
        }
    }
}

struct ba_matcher_state
{
    SHARED
    long long i, b;
    long long ba[];
};

static inline bool mismatch(long long i, struct ba_matcher_state *s)
{
    // We can't have s->b == m(s) if p has a sentinel, but
    // otherwise we could. We are not assuming a sentinel here,
    // so we need to explicitly consider this a mismatch.
    return (s->b == m(s)) || (x(s)[i] != p(s)[s->b]);
}

static long long ba_next(struct ba_matcher_state *s)
{
    for (long long i = s->i; i < n(s); ++i)
    {
        while (s->b > 0 && mismatch(i, s))
            s->b = s->ba[s->b - 1];
        s->b = (x(s)[i] == p(s)[s->b]) ? s->b + 1 : 0;
        if (s->b == m(s))
        {
            s->i = i + 1;
            return i - m(s) + 1;
        }
    }

    return -1;
}

static cstr_exact_matcher_vtab ba_vtab = {MATCHER_VTAB(ba_next, free)};
cstr_exact_matcher *cstr_ba_matcher(cstr_const_sslice x, cstr_const_sslice p)
{
    // allocate space for the the struct + the border array
    // in the flexible array for ba.
    struct ba_matcher_state *state =
        CSTR_MALLOC_FLEX_ARRAY(state, ba, (size_t)p.len);
    *state = (struct ba_matcher_state){
        MATCHER(ba_vtab, x, p), .i = 0, .b = 0};
    compute_border_array(p, state->ba);
    return (cstr_exact_matcher *)state;
}

// KMP O(n+m)

struct kmp_matcher_state
{
    SHARED
    long long i, j;
    long long ba[];
};

static long long kmp_next(struct kmp_matcher_state *s)
{
    long long i = s->i;
    long long j = s->j;

    for (; i < n(s); ++i)
    {
        // move pattern down...
        while (j > 0 && x(s)[i] != p(s)[j])
        {
            j = s->ba[j - 1];
        }

        // match...
        if (x(s)[i] == p(s)[j])
        {
            j++;
            if (j == m(s))
            {
                // we have a match!
                s->j = s->ba[j - 1];
                s->i = i + 1;
                return i - m(s) + 1;
            }
        }
    }

    return -1;
}

static cstr_exact_matcher_vtab kmp_vtab = {MATCHER_VTAB(kmp_next, free)};
cstr_exact_matcher *cstr_kmp_matcher(cstr_const_sslice x, cstr_const_sslice p)
{
    struct kmp_matcher_state *state =
        CSTR_MALLOC_FLEX_ARRAY(state, ba, (size_t)p.len);
    *state = (struct kmp_matcher_state){
        MATCHER(kmp_vtab, x, p),
        .i = 0, .j = 0};
    compute_border_array(p, state->ba);
    return (cstr_exact_matcher *)state;
}

// while these are only defined in this compilation unit, and will
// go out of scope now anyway, I just get rid of them to clean up.
// You never know what I might add below here later...
#undef x
#undef p
#undef n
#undef m
