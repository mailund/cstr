#include "cstr.h"

static inline uint8_t sentinel_idx(cstr_const_sslice x, long long i)
{
    return (i < x.len) ? x.buf[i] : 0;
}

typedef struct
{
    long long offset;
    long long lo, hi;
} block;

static long long lower(long long lo, long long hi, long long offset,
                       uint8_t a, cstr_const_sslice x, cstr_suffix_array sa)
{
    while (lo < hi)
    {
        long long m = (lo + hi) / 2;
        if (sentinel_idx(x, sa.buf[m] + offset) < a)
        {
            lo = m + 1;
        }
        else
        {
            hi = m;
        }
    }
    return lo;
}

static inline long long upper(long long lo, long long hi, long long offset,
                              uint8_t a, cstr_const_sslice x, cstr_suffix_array sa)
{
    return lower(lo, hi, offset, (uint8_t)(a + 1), x, sa);
}

static inline void update_block(long long *lo, long long *hi, long long *offset,
                                uint8_t a, cstr_const_sslice x, cstr_suffix_array sa)
{
    *lo = lower(*lo, *hi, *offset, a, x, sa);
    *hi = upper(*lo, *hi, *offset, a, x, sa);
    (*offset)++;
}

typedef struct sa_matcher
{
    cstr_exact_matcher matcher;
    cstr_suffix_array sa;
    long long next;
    long long end;
} sa_matcher;

static long long next_match(sa_matcher *m)
{
    return (m->next == m->end) ? -1 : (long long)m->sa.buf[m->next++];
}

typedef long long (*next_f)(cstr_exact_matcher *);
typedef void (*free_f)(cstr_exact_matcher *);
static cstr_exact_matcher_vtab sa_matcher_vtab = {.next = (next_f)next_match, .free = (free_f)free};
cstr_exact_matcher *cstr_sa_bsearch(cstr_suffix_array sa, cstr_const_sslice x, cstr_const_sslice p)
{
    sa_matcher *m = cstr_malloc(sizeof *m);
    m->matcher = (cstr_exact_matcher){.vtab = &sa_matcher_vtab};
    m->sa = sa;
    m->next = 0;
    m->end = sa.len;

    for (long long i = 0, offset = 0; i < p.len; i++)
    {
        update_block(&m->next, &m->end, &offset, p.buf[i], x, sa);
        if (m->next == m->end)
        {
            break;
        }
    }

    return (cstr_exact_matcher *)m;
}
