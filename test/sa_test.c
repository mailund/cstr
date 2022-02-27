#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "testlib.h"
#include <cstr.h>

static TL_PARAM_TEST(check_suffix_ordered,
                     uint8_t const *x, cstr_suffix_array sa)
{
    TL_BEGIN();
    for (int i = 1; i < sa.len; i++)
    {
        //printf("sa[%d] == %d %s\n", i - 1, sa.buf[i - 1], x + sa.buf[i - 1]);
        //printf("sa[%d] == %d %s\n", i, sa.buf[i], x + sa.buf[i]);
        TL_ERROR_IF_GE_STRING(x + sa.buf[i - 1], x + sa.buf[i]);
        //printf("OK\n");
    }
    TL_END();
}

typedef void (*const_alg)(cstr_suffix_array sa, cstr_const_uislice x, cstr_alphabet *alpha);
static TL_PARAM_TEST(test_mississippi, const_alg alg)
{
    TL_BEGIN();

    cstr_const_sslice x = CSTR_SLICE_STRING0((const char *)"mississippi");

    cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, x);

    cstr_uislice *mapped = cstr_alloc_uislice(x.len);
    // since alpha was created from x we cannot get mapping errors
    // here
    cstr_alphabet_map_to_uint(*mapped, x, &alpha);
    assert(mapped->buf); // for static analyser

    cstr_suffix_array *sa = cstr_alloc_uislice(x.len);
    assert(sa->buf); // For the static analyser

    alg(*sa, CSTR_SLICE_CONST_CAST(*mapped), &alpha);

    TL_RUN_PARAM_TEST(check_suffix_ordered, "mississippi", x.buf, *sa);

    free(mapped);
    free(sa);

    TL_END();
}

static TL_PARAM_TEST(test_random, const_alg alg)
{
    TL_BEGIN();

    const long long n = 10;

    cstr_const_sslice letters = CSTR_SLICE_STRING0((const char *)"acgt");
    cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, letters);

    cstr_sslice *x = cstr_alloc_sslice(n);
    cstr_uislice *mapped = cstr_alloc_uislice(n);
    cstr_suffix_array *sa = cstr_alloc_uislice(n);

    assert(x->buf);      // For the static analyser
    assert(mapped->buf); // For the static analyser
    assert(sa->buf);     // For the static analyser

    for (int k = 0; k < 10; k++)
    {
        // len-1 since we don't want to sample the sentinel
        tl_random_string0(*x, letters.buf, (int)letters.len - 1);
        bool ok = cstr_alphabet_map_to_uint(*mapped, CSTR_SLICE_CONST_CAST(*x), &alpha);
        TL_ERROR_IF(!ok);
        alg(*sa, CSTR_SLICE_CONST_CAST(*mapped), &alpha);

        TL_RUN_PARAM_TEST(check_suffix_ordered, "random", x->buf, *sa);
    }

    free(sa);
    free(mapped);
    free(x);

    TL_END();
}

int main(void)
{
    TL_BEGIN_TEST_SUITE("sa_test");
    TL_RUN_PARAM_TEST(test_mississippi, "skew", cstr_skew);
    TL_RUN_PARAM_TEST(test_mississippi, "sais", cstr_sais);
    TL_RUN_PARAM_TEST(test_random, "skew", cstr_skew);
    TL_RUN_PARAM_TEST(test_random, "sais", cstr_sais);
    TL_END_SUITE();
}
