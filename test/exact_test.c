#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>

#include "testlib.h"

typedef struct cstr_exact_matcher *(*algorithm_fn)(cstr_sslice, cstr_sslice);

TL_PARAM_TEST(test_simple_cases_p, algorithm_fn f)
{
    TL_BEGIN();
    {
        char *x = "aaba";
        char *p = "a";
        cstr_exact_matcher *m = f(CSTR_SSLICE_STRING(x), CSTR_SSLICE_STRING(p));
        int i = cstr_exact_next_match(m);
        TL_ERROR_IF_NEQ_INT(i, 0);
        i = cstr_exact_next_match(m);
        TL_ERROR_IF_NEQ_INT(i, 1);
        i = cstr_exact_next_match(m);
        TL_ERROR_IF_NEQ_INT(i, 3);
        i = cstr_exact_next_match(m);
        TL_ERROR_IF_NEQ_INT(i, -1);
        cstr_free_exact_matcher(m);
    }
    {
        char *x = "abab";
        char *p = "ab";
        cstr_exact_matcher *m = f(CSTR_SSLICE_STRING(x), CSTR_SSLICE_STRING(p));
        int i = cstr_exact_next_match(m);
        TL_ERROR_IF_NEQ_INT(i, 0);
        i = cstr_exact_next_match(m);
        TL_ERROR_IF_NEQ_INT(i, 2);
        i = cstr_exact_next_match(m);
        TL_ERROR_IF_NEQ_INT(i, -1);
        cstr_free_exact_matcher(m);
    }
    {
        char *x = "aaaa";
        char *p = "aa";
        cstr_exact_matcher *m = f(CSTR_SSLICE_STRING(x), CSTR_SSLICE_STRING(p));
        int i = cstr_exact_next_match(m);
        TL_ERROR_IF_NEQ_INT(i, 0);
        i = cstr_exact_next_match(m);
        TL_ERROR_IF_NEQ_INT(i, 1);
        i = cstr_exact_next_match(m);
        TL_ERROR_IF_NEQ_INT(i, 2);
        i = cstr_exact_next_match(m);
        TL_ERROR_IF_NEQ_INT(i, -1);
        cstr_free_exact_matcher(m);
    }
    TL_END();
}

TL_PARAM_TEST(test_random_string_p, algorithm_fn f)
{
    TL_BEGIN();
    
    cstr_sslice x = CSTR_ALLOC_SSLICE(100);
    cstr_sslice p = CSTR_ALLOC_SSLICE(5);
    
    for (int i = 0; i < 10; i++) {
        tl_random_string(x, "abc", 3);
        for (int j = 0; j < 10; j++) {
            tl_random_string(p, "abc", 3);
            
            cstr_exact_matcher *matcher = f(x, p);
            for (int m = cstr_exact_next_match(matcher);
                 m != -1;
                 m = cstr_exact_next_match(matcher)) {
                cstr_sslice match = CSTR_SSLICE(x.buf + m, p.len);
                TL_ERROR_IF(!cstr_sslice_eq(match, p));
            }
            cstr_free_exact_matcher(matcher);
            
        }
    }
    
    CSTR_FREE_SLICE_BUFFER(p);
    CSTR_FREE_SLICE_BUFFER(x);
    
    TL_END();
}

TL_TEST(simple_test)
{
    TL_BEGIN();
    TL_RUN_PARAM_TEST(test_simple_cases_p, "naive", cstr_naive_matcher);
    TL_RUN_PARAM_TEST(test_simple_cases_p, "ba", cstr_ba_matcher);
    TL_RUN_PARAM_TEST(test_simple_cases_p, "kmp", cstr_kmp_matcher);
    TL_END();
}

TL_TEST(random_string)
{
    TL_BEGIN();
    TL_RUN_PARAM_TEST(test_random_string_p, "naive", cstr_naive_matcher);
    TL_RUN_PARAM_TEST(test_random_string_p, "ba", cstr_ba_matcher);
    TL_RUN_PARAM_TEST(test_random_string_p, "kmp", cstr_kmp_matcher);
    TL_END();
}

int main(void)
{
    TL_BEGIN_TEST_SUITE("exact_test");
    TL_RUN_TEST(simple_test);
    TL_RUN_TEST(random_string);
    TL_END_SUITE();
}
