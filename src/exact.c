#include <stdlib.h>
#include <string.h>

#include "cstr.h"
#include "cstr_internal.h"

typedef int (*exact_next_fn)(struct cstr_exact_matcher *);
typedef void (*exact_free_fn)(struct cstr_exact_matcher *);

struct cstr_exact_matcher {
    exact_next_fn next;
    exact_free_fn free;
};

// For embedding common string info in a state...
#define MATCH_CONTEXT                                                          \
    const char *x, *p;                                                         \
    int n, m;
#define EMBED_CONTEX(x, p)                                                     \
    .x = (x), .p = (p), .n = (int)strlen(x), .m = (int)strlen(p)

int cstr_exact_next_match(struct cstr_exact_matcher *matcher) {
    return matcher->next(matcher);
}

void cstr_free_exact_matcher(struct cstr_exact_matcher *matcher) {
    matcher->free(matcher);
}

// Naive O(nm) algorithm
struct naive_matcher_state {
    struct cstr_exact_matcher matcher;
    MATCH_CONTEXT
    int i;
};

static int naive_next(struct naive_matcher_state *state) {
    for (; state->i < state->n; state->i++) {
        for (int j = 0; j < state->m; j++) {
            if (state->x[state->i + j] != state->p[j])
                break;
            if (j == state->m - 1) {
                // a match
                state->i++; // next time, start from here
                return state->i - 1;
            }
        }
    }
    return -1; // If we get here, we are done.
}

struct cstr_exact_matcher *cstr_naive_matcher(const char *x, const char *p) {
    struct naive_matcher_state *state = malloc(sizeof *state);
    if (state) {
        *state = (struct naive_matcher_state){
            .matcher = {.next = (exact_next_fn)naive_next,
                        .free = (exact_free_fn)free},
            EMBED_CONTEX(x, p),
            .i = 0};
    }
    return (void *)state; // void cast to change type
}

// Border array algorithm O(n+m)

static void compute_border_array(const char *x, int m, int *ba) {
    // Border array
    ba[0] = 0;
    for (int i = 1; i < m; ++i) {
        int b = ba[i - 1];
        while (b > 0 && x[i] != x[b])
            b = ba[b - 1];
        ba[i] = (x[i] == x[b]) ? b + 1 : 0;
    }

    // restricted border array
    for (uint32_t i = 0; i < m - 1; i++) {
        if (ba[i] > 0 && x[ba[i]] == x[i + 1])
            ba[i] = ba[ba[i] - 1];
    }
}

struct ba_matcher_state {
    struct cstr_exact_matcher matcher;
    MATCH_CONTEXT
    int i, b;
    int *ba;
};

static int ba_next(struct ba_matcher_state *state) {
    int b = state->b;
    for (int i = state->i; i < state->n; ++i) {
        while (b > 0 && state->x[i] != state->p[b])
            b = state->ba[b - 1];
        b = (state->x[i] == state->p[b]) ? b + 1 : 0;
        if (b == state->m) {
            state->i = i + 1;
            state->b = b;
            return i - (int)state->m + 1;
        }
    }

    return -1;
}

static void ba_free(struct ba_matcher_state *state) {
    free(state->ba);
    free(state);
}

struct cstr_exact_matcher *cstr_ba_matcher(const char *x, const char *p) {
    struct ba_matcher_state *state = malloc(sizeof *state);
    if (state) {
        *state = (struct ba_matcher_state){
            .matcher = {.next = (exact_next_fn)ba_next,
                        .free = (exact_free_fn)ba_free},
            EMBED_CONTEX(x, p),
            .i = 0,
            .b = 0};
        state->ba = malloc((size_t)state->m * sizeof(state->ba[0]));
        if (!state->ba) {
            free(state);
            return 0;
        }
        compute_border_array(p, state->m, state->ba);
    }
    return (void *)state; // void cast to change type
}

// KMP O(n+m)

struct kmp_matcher_state {
    struct cstr_exact_matcher matcher;
    MATCH_CONTEXT
    int i, j;
    int *ba;
};

static int kmp_next(struct kmp_matcher_state *state) {
    int i = state->i;
    int j = state->j;
    for (; i < state->n; ++i) {
        // move pattern down...
        while (j > 0 && state->x[i] != state->p[j])
            j = state->ba[j - 1];

        // match...
        if (state->x[i] == state->p[j]) {
            j++;
            if (j == state->m) {
                // we have a match!
                state->j = state->ba[j - 1];
                state->i = i + 1;
                return i - state->m + 1;
            }
        }
    }

    return -1;
}

static void kmp_free(struct kmp_matcher_state *state) {
    free(state->ba);
    free(state);
}

struct cstr_exact_matcher *cstr_kmp_matcher(const char *x, const char *p) {
    struct kmp_matcher_state *state = malloc(sizeof *state);
    if (state) {
        *state = (struct kmp_matcher_state){
            .matcher = {.next = (exact_next_fn)kmp_next,
                        .free = (exact_free_fn)kmp_free},
            EMBED_CONTEX(x, p),
            .i = 0,
            .j = 0};
        state->ba = malloc((size_t)state->m * sizeof(state->ba[0]));
        if (!state->ba) {
            free(state);
            return 0;
        }
        compute_border_array(p, state->m, state->ba);
    }
    return (void *)state; // void cast to change type
}
