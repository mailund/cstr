#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bwt_internal.h"

struct c_table *cstr_build_c_table(cstr_const_sslice x, long long sigma)
{
    struct c_table *ctab = CSTR_MALLOC_FLEX_ARRAY(ctab, cumsum, (size_t)sigma);
    ctab->sigma = sigma;
    for (long long i = 0; i < sigma; i++)
        ctab->cumsum[i] = 0;
    for (long long i = 0; i < x.len; i++)
        ctab->cumsum[x.buf[i]]++;
    for (long long i = 0, acc = 0; i < sigma; i++)
    {
        unsigned int k = ctab->cumsum[i];
        ctab->cumsum[i] = (unsigned int)acc;
        acc += k;
    }
    return ctab;
}

struct o_table *cstr_build_o_table(cstr_const_sslice bwt, struct c_table const *ctab)
{
    struct o_table *otab = CSTR_MALLOC_FLEX_ARRAY(otab, table, (size_t)ctab->sigma * (size_t)bwt.len);
    otab->sigma = ctab->sigma;
    otab->n = bwt.len;
    for (long long a = 0; a < ctab->sigma; a++)
    {
        O_RAW(a, 0) = (a == bwt.buf[0]);
        for (int i = 1; i < bwt.len; i++)
        {
            O_RAW(a, i) = O_RAW(a, i - 1) + (a == bwt.buf[i]);
        }
    }
    return otab;
}

void cstr_bwt(cstr_sslice bwt, cstr_const_sslice x, cstr_suffix_array sa)
{
    assert(bwt.len == x.len && x.len == sa.len);
    for (long long i = 0; i < x.len; i++)
    {
        // Previous index, with wrap at index zero...
        long long j = sa.buf[i] + (sa.buf[i] == 0) * x.len;
        bwt.buf[i] = x.buf[j - 1];
    }
}

void cstr_reverse_bwt(cstr_sslice rev, cstr_const_sslice bwt, cstr_suffix_array sa)
{
    assert(rev.len == bwt.len && bwt.len == sa.len);

    // Use the alphabet code to get the size of the alphabet, sigma.
    cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, bwt);
    long long sigma = alpha.size;

    // We don't assume that the bwt string is mapped down to the alphabet here (although
    // maybe we should to save some time), so we also need to do that...
    cstr_sslice *mapped_buf = cstr_alloc_sslice(bwt.len);
    cstr_alphabet_map(*mapped_buf, bwt, &alpha);
    cstr_const_sslice mapped = CSTR_SLICE_CONST_CAST(*mapped_buf);

    struct c_table *ctab = cstr_build_c_table(mapped, sigma);
    struct o_table *otab = cstr_build_o_table(mapped, ctab);

    rev.buf[rev.len - 1] = 0; // Sentinel at the end of rev.
    // The sentinel is also at the beginning of row 0 in the BWT
    // matrix, so i starts at zero.
    for (long long i = 0, j = rev.len - 2; j >= 0; j--)
    {
        uint8_t a = mapped.buf[i];
        rev.buf[j] = a;
        i = C(a) + O(a, i);
    }

    // If the input wasn't mapped down to the alphabet, then rev isn't in the
    // right alphabet either, and so we need to map it back.
    cstr_alphabet_revmap(rev, CSTR_SLICE_CONST_CAST(rev), &alpha);

    free(mapped_buf);
    free(ctab);
    free(otab);
}

struct cstr_bwt_preproc
{
    cstr_alphabet alpha;
    cstr_suffix_array *sa;
    struct c_table *ctab;
    struct o_table *otab;
};

struct cstr_bwt_preproc *cstr_bwt_preprocess(cstr_const_sslice x)
{
    struct cstr_bwt_preproc *preproc = cstr_malloc(sizeof *preproc);
    cstr_init_alphabet(&preproc->alpha, x);

    // We don't assume that the bwt string is mapped down to the alphabet here (although
    // maybe we should to save some time), so we also need to do that... Anyway, the
    // mapping is needed both for constructing the suffix array and for representing
    // the tables.

    // Map the string into an integer slice so we can build the suffix array.
    cstr_uislice *u_buf = cstr_alloc_uislice(x.len);
    cstr_alphabet_map_to_uint(*u_buf, x, &preproc->alpha);
    cstr_const_uislice u = CSTR_SLICE_CONST_CAST(*u_buf);

    // Then build the suffix array
    preproc->sa = cstr_alloc_uislice(x.len);
    cstr_sais(*preproc->sa, u, &preproc->alpha);

    // From the suffix array we can build the BWT of x
    cstr_sslice *w_buf = cstr_alloc_sslice(x.len);
    cstr_alphabet_map(*w_buf, x, &preproc->alpha);
    cstr_const_sslice w = CSTR_SLICE_CONST_CAST(*w_buf);

    cstr_sslice *bwt_buf = cstr_alloc_sslice(x.len);
    cstr_bwt(*bwt_buf, w, *preproc->sa);
    cstr_const_sslice bwt = CSTR_SLICE_CONST_CAST(*bwt_buf);

    // With the BWT in hand, we can build the tables.
    preproc->ctab = cstr_build_c_table(bwt, preproc->alpha.size);
    preproc->otab = cstr_build_o_table(bwt, preproc->ctab);

    // We don't need the mapped string nor the BWT any more.
    // The information we need is all in the tables in preproc.
    free(bwt_buf);
    free(w_buf);
    free(u_buf);

    return preproc;
}

void cstr_free_bwt_preproc(struct cstr_bwt_preproc *preproc)
{
    free(preproc->sa);
    free(preproc->ctab);
    free(preproc->otab);
}

typedef struct fmindex_matcher
{
    cstr_exact_matcher matcher;
    cstr_bwt_preproc *preproc;
    long long next;
    long long end;
} fmindex_matcher;

static long long next_match(fmindex_matcher *m)
{
    if (m->next == m->end)
    {
        return -1;
    }
    return m->preproc->sa->buf[m->next++];
}

typedef long long (*next_f)(cstr_exact_matcher *);
typedef void (*free_f)(cstr_exact_matcher *);
static cstr_exact_matcher_vtab bwt_matcher_vtab = {.next = (next_f)next_match, .free = (free_f)free};

cstr_exact_matcher *
cstr_fmindex_search(cstr_bwt_preproc *preproc, cstr_const_sslice raw_p)
{
    // If we cannot map p, this will be what we return; it is an
    // empty interval.
    long long left = 0, right = 0;

    cstr_sslice *p_buf = cstr_alloc_sslice(raw_p.len);
    bool ok = cstr_alphabet_map(*p_buf, raw_p, &preproc->alpha);
    if (ok)
    {
        cstr_const_sslice p = CSTR_SLICE_CONST_CAST(*p_buf);
        struct c_table *ctab = preproc->ctab;
        struct o_table *otab = preproc->otab;

        left = 0;
        right = otab->n;
        for (long long i = p.len - 1; i >= 0; i--)
        {
            uint8_t a = p.buf[i];
            left = C(a) + O(a, left);
            right = C(a) + O(a, right);
            if (left >= right)
            {
                break;
            }
        }
    }
    free(p_buf);

    fmindex_matcher *m = cstr_malloc(sizeof *m);
    *m = (fmindex_matcher){
        .matcher = {.vtab = &bwt_matcher_vtab},
        .preproc = preproc,
        .next = left,
        .end = right,
    };

    return (cstr_exact_matcher *)m;
}
