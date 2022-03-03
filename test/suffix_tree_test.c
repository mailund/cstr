#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unittests.h"
#include <testlib.h>

#define next cstr_match_next
static TL_PARAM_TEST(check_suffix_ordered_for_construction,
                     cstr_const_sslice x,
                     cstr_suffix_tree *st)
{
    TL_BEGIN();
    cstr_exact_matcher *m = cstr_st_exact_search(st, CSTR_SLICE_STRING((const char *)""));
    
    long long count = 1;
    cstr_const_sslice prev = CSTR_SUFFIX(x, cstr_exact_next_match(m));
    for (long long suf_idx = cstr_exact_next_match(m);
         suf_idx != -1;
         suf_idx = cstr_exact_next_match(m))
    {
        count++;
        cstr_const_sslice suf = CSTR_SUFFIX(x, suf_idx);
        TL_ERROR_IF_GE_SLICE(prev, suf);
        prev = suf;
    }
    TL_FATAL_IF_NEQ_LL(count, x.len); // We should see all suffixes

    cstr_free_exact_matcher(m);
    TL_END();
}
#undef next

static TL_TEST(check_suffix_ordered)
{
    TL_BEGIN();

    const long long n = 50;

    cstr_alphabet alpha;
    cstr_const_sslice letters = CSTR_SLICE_STRING0((const char *)"acgt");
    cstr_init_alphabet(&alpha, letters);

    cstr_sslice *orig = cstr_alloc_sslice(n);
    cstr_sslice *x = cstr_alloc_sslice(n);

    for (int k = 0; k < 10; k++)
    {
        // len-1 since we don't want to sample the sentinel
        tl_random_string0(*orig, letters.buf, (int)letters.len - 1);
        bool ok = cstr_alphabet_map(*x, CSTR_SLICE_CONST_CAST(*orig), &alpha);
        TL_ERROR_IF(!ok);

        cstr_suffix_tree *st = cstr_naive_suffix_tree(&alpha, CSTR_SLICE_CONST_CAST(*x));
        TL_RUN_PARAM_TEST(check_suffix_ordered_for_construction, "naive", CSTR_SLICE_CONST_CAST(*x), st);
        cstr_free_suffix_tree(st);

        st = cstr_mccreight_suffix_tree(&alpha, CSTR_SLICE_CONST_CAST(*x));
        TL_RUN_PARAM_TEST(check_suffix_ordered_for_construction, "mccreight", CSTR_SLICE_CONST_CAST(*x), st);
        cstr_free_suffix_tree(st);
    }

    free(x);
    free(orig);

    TL_END();
}

int main(void)
{
    TL_BEGIN_TEST_SUITE("suffix tree test");
    // Unit tests
    TL_RUN_TEST(st_constructing_leaves);
    TL_RUN_TEST(st_constructing_inner_nodes);
    TL_RUN_TEST(st_attempted_scans);
    TL_RUN_TEST(st_dft);
    TL_RUN_TEST(st_search);

    // Interface tests
    TL_RUN_TEST(check_suffix_ordered);

    TL_END_SUITE();
}
