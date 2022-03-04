#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>

#include "testlib.h"

typedef cstr_exact_matcher *(*algorithm_fn)(cstr_const_sslice, cstr_const_sslice);

static TL_PARAM_TEST(test_simple_cases_p, algorithm_fn f)
{
    TL_BEGIN();
    {
        const char *x = "aaba";
        const char *p = "a";
        cstr_exact_matcher *m = f(CSTR_SLICE_STRING(x), CSTR_SLICE_STRING(p));
        long long i = cstr_exact_next_match(m);
        TL_ERROR_IF_NEQ_LL(i, 0LL);
        i = cstr_exact_next_match(m);
        TL_ERROR_IF_NEQ_LL(i, 1LL);
        i = cstr_exact_next_match(m);
        TL_ERROR_IF_NEQ_LL(i, 3LL);
        i = cstr_exact_next_match(m);
        TL_ERROR_IF_NEQ_LL(i, -1LL);
        cstr_free_exact_matcher(m);
    }
    {
        const char *x = "abab";
        const char *p = "ab";
        cstr_exact_matcher *m = f(CSTR_SLICE_STRING(x), CSTR_SLICE_STRING(p));
        long long i = cstr_exact_next_match(m);
        TL_ERROR_IF_NEQ_LL(i, 0LL);
        i = cstr_exact_next_match(m);
        TL_ERROR_IF_NEQ_LL(i, 2LL);
        i = cstr_exact_next_match(m);
        TL_ERROR_IF_NEQ_LL(i, -1LL);
        cstr_free_exact_matcher(m);
    }
    {
        const char *x = "aaaa";
        const char *p = "aa";
        cstr_exact_matcher *m = f(CSTR_SLICE_STRING(x), CSTR_SLICE_STRING(p));
        long long i = cstr_exact_next_match(m);
        TL_ERROR_IF_NEQ_LL(i, 0LL);
        i = cstr_exact_next_match(m);
        TL_ERROR_IF_NEQ_LL(i, 1LL);
        i = cstr_exact_next_match(m);
        TL_ERROR_IF_NEQ_LL(i, 2LL);
        i = cstr_exact_next_match(m);
        TL_ERROR_IF_NEQ_LL(i, -1LL);
        cstr_free_exact_matcher(m);
    }
    TL_END();
}

static TL_PARAM_TEST(test_random_string_p, algorithm_fn f)
{
    TL_BEGIN();

    cstr_sslice *x = cstr_alloc_sslice(100);
    cstr_sslice *p = cstr_alloc_sslice(5);

    for (int i = 0; i < 10; i++)
    {
        tl_random_string(*x, (const uint8_t *)"abc", 3);
        for (int j = 0; j < 10; j++)
        {
            tl_random_string(*p, (const uint8_t *)"abc", 3);

            cstr_exact_matcher *matcher = f(CSTR_SLICE_CONST_CAST(*x), CSTR_SLICE_CONST_CAST(*p));
            for (long long m = cstr_exact_next_match(matcher);
                 m != -1;
                 m = cstr_exact_next_match(matcher))
            {
                cstr_sslice match = CSTR_SUBSLICE(*x, m, m + p->len);
                TL_ERROR_IF(!CSTR_SLICE_EQ(match, *p));
            }
            cstr_free_exact_matcher(matcher);
        }
    }

    free(p);
    free(x);

    TL_END();
}

static TL_PARAM_TEST(test_prefix_p, algorithm_fn f)
{
    TL_BEGIN();

    cstr_sslice *x = cstr_alloc_sslice(100);

    for (int i = 0; i < 10; i++)
    {
        tl_random_string(*x, (uint8_t const *)"abc", 3);
        for (int j = 0; j < 10; j++)
        {
            cstr_sslice p = tl_random_prefix(*x);
            cstr_exact_matcher *matcher = f(CSTR_SLICE_CONST_CAST(*x), CSTR_SLICE_CONST_CAST(p));
            TL_ERROR_IF_NEQ_LL(0LL, cstr_exact_next_match(matcher));
            cstr_free_exact_matcher(matcher);
        }
    }

    free(x);

    TL_END();
}

static TL_PARAM_TEST(test_suffix_p, algorithm_fn f)
{
    TL_BEGIN();

    cstr_sslice *x = cstr_alloc_sslice(100);

    for (int i = 0; i < 10; i++)
    {
        tl_random_string(*x, (uint8_t const *)"abc", 3);
        for (int j = 0; j < 10; j++)
        {
            cstr_sslice p = tl_random_suffix(*x);
            cstr_exact_matcher *matcher = f(CSTR_SLICE_CONST_CAST(*x), CSTR_SLICE_CONST_CAST(p));

            long long res = cstr_exact_next_match(matcher);
            long long last = res;
            while (res != -1LL)
            {
                last = res;
                res = cstr_exact_next_match(matcher);
            }

            TL_ERROR_IF_NEQ_LL((long long)last, x->len - p.len);

            cstr_free_exact_matcher(matcher);
        }
    }

    free(x);

    TL_END();
}

static TL_TEST(simple_test)
{
    TL_BEGIN();
    TL_RUN_PARAM_TEST(test_simple_cases_p, "naive", cstr_naive_matcher);
    TL_RUN_PARAM_TEST(test_simple_cases_p, "ba", cstr_ba_matcher);
    TL_RUN_PARAM_TEST(test_simple_cases_p, "kmp", cstr_kmp_matcher);
    TL_END();
}

static TL_TEST(test_random_string)
{
    TL_BEGIN();
    TL_RUN_PARAM_TEST(test_random_string_p, "naive", cstr_naive_matcher);
    TL_RUN_PARAM_TEST(test_random_string_p, "ba", cstr_ba_matcher);
    TL_RUN_PARAM_TEST(test_random_string_p, "kmp", cstr_kmp_matcher);
    TL_END();
}

static TL_TEST(test_prefix)
{
    TL_BEGIN();
    TL_RUN_PARAM_TEST(test_prefix_p, "naive", cstr_naive_matcher);
    TL_RUN_PARAM_TEST(test_prefix_p, "ba", cstr_ba_matcher);
    TL_RUN_PARAM_TEST(test_prefix_p, "kmp", cstr_kmp_matcher);
    TL_END();
}

static TL_TEST(test_suffix)
{
    TL_BEGIN();
    TL_RUN_PARAM_TEST(test_suffix_p, "naive", cstr_naive_matcher);
    TL_RUN_PARAM_TEST(test_suffix_p, "ba", cstr_ba_matcher);
    TL_RUN_PARAM_TEST(test_suffix_p, "kmp", cstr_kmp_matcher);
    TL_END();
}

int main(void)
{
    TL_BEGIN_TEST_SUITE("exact_test");
    TL_RUN_TEST(simple_test);
    TL_RUN_TEST(test_random_string);
    TL_RUN_TEST(test_prefix);
    TL_RUN_TEST(test_suffix);
    TL_END_SUITE();
}
