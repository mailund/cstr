#include <stdlib.h>
#include <string.h>

#include "cstr.h"
#include "cstr_internal.h"

typedef int (*exact_next_fn)(struct cstr_exact_matcher *);
typedef void (*exact_free_fn)(struct cstr_exact_matcher *);

struct cstr_exact_matcher
{
    exact_next_fn next;
    exact_free_fn free;
    csslice x, p;
};

// Helper macro for initialising the matcher header
#define MATCHER(NEXT, FREE, X, P)          \
    .matcher = (struct cstr_exact_matcher) \
    {                                      \
        .next = (exact_next_fn)(NEXT),     \
        .free = (exact_free_fn)(FREE),     \
        .x = (X),                          \
        .p = (P)                           \
    }

// polymorphic interface
int cstr_exact_next_match(struct cstr_exact_matcher *matcher)
{
    return matcher->next(matcher);
}

void cstr_free_exact_matcher(struct cstr_exact_matcher *matcher)
{
    matcher->free(matcher);
}

// macros for readability
#define x(S) ((S)->matcher.x.buf)
#define p(S) ((S)->matcher.p.buf)
#define n(S) ((int)(S)->matcher.x.len)
#define m(S) ((int)(S)->matcher.p.len)

// Naive O(nm) algorithm
struct naive_matcher_state
{
    struct cstr_exact_matcher matcher;
    int i;
};

static int naive_next(struct naive_matcher_state *s)
{
    for (; s->i < n(s); s->i++)
    {
        for (int j = 0; j < m(s); j++)
        {
            if (x(s)[s->i + j] != p(s)[j])
                break;
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

struct cstr_exact_matcher *
cstr_naive_matcher(csslice x, csslice p)
{
    struct naive_matcher_state *state = malloc(sizeof *state);
    if (state)
    {
        // init struct, then move it, to get around const
        struct naive_matcher_state data = {
            MATCHER(naive_next, free, x, p),
            .i = 0};
        memcpy(state, &data, sizeof data);
    }
    return (void *)state; // void cast to change type
}

// Border array algorithm O(n+m)

static void compute_border_array(csslice p, int *ba)
{
    // Border array
    ba[0] = 0;
    for (int i = 1; i < p.len; ++i)
    {
        int b = ba[i - 1];
        while (b > 0 && p.buf[i] != p.buf[b])
            b = ba[b - 1];
        ba[i] = (p.buf[i] == p.buf[b]) ? b + 1 : 0;
    }

    // restricted border array
    for (uint32_t i = 0; i < p.len - 1; i++)
    {
        if (ba[i] > 0 && p.buf[ba[i]] == p.buf[i + 1])
            ba[i] = ba[ba[i] - 1];
    }
}

struct ba_matcher_state
{
    struct cstr_exact_matcher matcher;
    int i, b;
    int ba[];
};

static int ba_next(struct ba_matcher_state *s)
{
    for (int i = s->i; i < n(s); ++i)
    {
        while (s->b > 0 && x(s)[i] != p(s)[s->b])
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

struct cstr_exact_matcher *
cstr_ba_matcher(csslice x, csslice p)
{
    // allocate space for the the struct + the border array
    // in the flexible array for ba.
    struct ba_matcher_state *state =
        malloc(sizeof *state + p.len * sizeof(state->ba[0]));
    if (state)
    {
        // init struct, then move it, to get around const
        struct ba_matcher_state data = {
            MATCHER(ba_next, free, x, p),
            .i = 0, .b = 0};
        // move sizeof data moves data up to ba, leaving ba
        // to be filled out afterwards.
        memcpy(state, &data, sizeof data);
        compute_border_array(p, state->ba);
    }
    return (void *)state; // void cast to change type
}

// KMP O(n+m)

struct kmp_matcher_state
{
    struct cstr_exact_matcher matcher;
    int i, j;
    int ba[];
};

static int kmp_next(struct kmp_matcher_state *s)
{
    int i = s->i;
    int j = s->j;

    for (; i < n(s); ++i)
    {
        // move pattern down...
        while (j > 0 && x(s)[i] != p(s)[j])
            j = s->ba[j - 1];

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

struct cstr_exact_matcher *cstr_kmp_matcher(csslice x, csslice p)
{
    struct kmp_matcher_state *state =
        malloc(sizeof *state + p.len * sizeof(state->ba[0]));
    if (state)
    {
        // init struct, then move it, to get around const
        struct kmp_matcher_state data = {
            MATCHER(kmp_next, free, x, p),
            .i = 0, .j = 0};
        // move sizeof data moves data up to ba, leaving ba
        // to be filled out afterwards.
        memcpy(state, &data, sizeof data);
        compute_border_array(p, state->ba);
    }
    return (void *)state; // void cast to change type
}

// while these are only defined in this compilation unit, and will
// go out of scope now anyway, I just get rid of them to clean up.
// You never know what I might add below here later...
#undef x
#undef p
#undef n
#undef m