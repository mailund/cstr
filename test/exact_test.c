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
        struct cstr_exact_matcher *m = f(CSTR_SSLICE_STRING(x), CSTR_SSLICE_STRING(p));
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
        struct cstr_exact_matcher *m = f(CSTR_SSLICE_STRING(x), CSTR_SSLICE_STRING(p));
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
        struct cstr_exact_matcher *m = f(CSTR_SSLICE_STRING(x), CSTR_SSLICE_STRING(p));
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
    TL_END();
}

#if 0
int main(void) {
    const char *alpha = "acgt";
    int alpha_len = strlen(alpha);
    char buf[100];
    tl_random_string(alpha, alpha_len, buf, 100);
    for (int i = 0; i < 100; i++) {
        putchar(buf[i]);
    }
    putchar('\n');

    return 0;
}
#endif

TL_TEST(simple_test)
{
    TL_BEGIN();
    TL_RUN_PARAM_TEST(test_simple_cases_p, "naive", cstr_naive_matcher);
    TL_RUN_PARAM_TEST(test_simple_cases_p, "ba", cstr_ba_matcher);
    TL_RUN_PARAM_TEST(test_simple_cases_p, "kmp", cstr_kmp_matcher);
    TL_END();
}

int main(void)
{
    TL_BEGIN_TEST_SUITE("exact_test");
    TL_RUN_TEST(simple_test);
    TL_END_SUITE();
}
