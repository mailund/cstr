#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "testlib.h"
#include <cstr.h>

TL_PARAM_TEST(check_suffix_ordered,
              char const *x, cstr_suffix_array sa)
{
    TL_BEGIN();
    for (int i = 1; i < sa.len; i++)
    {
        printf("sa[%d] == %d %s\n", i, sa.buf[i - 1], x + sa.buf[i - 1]);
        printf("sa[%d] == %d %s\n", i, sa.buf[i], x + sa.buf[i]);
        TL_ERROR_IF_GE_STRING(x + sa.buf[i - 1], x + sa.buf[i]);
    }
    TL_END();
}

TL_TEST(test_mississippi)
{
    TL_BEGIN();

    cstr_sslice x = CSTR_SLICE_STRING0("mississippi");

    cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, x);

    cstr_uislice mapped = CSTR_ALLOC_SLICE_BUFFER(mapped, x.len);
    // since alpha was created from x we cannot get mapping errors
    // here
    cstr_alphabet_map_to_uint(mapped, x, &alpha);
    assert(mapped.buf); // for static analyser

    cstr_suffix_array sa = CSTR_ALLOC_SLICE_BUFFER(sa, x.len);
    assert(sa.buf); // For the static analyser

    cstr_skew(sa, mapped, &alpha);

    TL_RUN_PARAM_TEST(check_suffix_ordered, "mississippi", x.buf, sa);

    CSTR_FREE_SLICE_BUFFER(sa);

    TL_END();
}

TL_TEST(test_random)
{
    TL_BEGIN();

    const long long n = 10;

    cstr_sslice letters = CSTR_SLICE_STRING("acgt");
    cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, letters);

    cstr_sslice x = CSTR_ALLOC_SLICE_BUFFER(x, n);
    cstr_uislice mapped = CSTR_ALLOC_SLICE_BUFFER(mapped, n);
    cstr_suffix_array sa = CSTR_ALLOC_SLICE_BUFFER(sa, n);

    assert(x.buf);      // For the static analyser
    assert(mapped.buf); // for static analyser
    assert(sa.buf);     // For the static analyser

    for (int k = 0; k < 10; k++)
    {
        // len-1 since we don't want to overwrite sentinel.
        tl_random_string0(x, letters.buf, letters.len);
        cstr_alphabet_map_to_uint(mapped, x, &alpha);
        cstr_skew(sa, mapped, &alpha);

        TL_RUN_PARAM_TEST(check_suffix_ordered, "random", x.buf, sa);
    }

    CSTR_FREE_SLICE_BUFFER(sa);
    CSTR_FREE_SLICE_BUFFER(mapped);
    CSTR_FREE_SLICE_BUFFER(x);

    TL_END();
}

int main(void)
{
    TL_BEGIN_TEST_SUITE("sa_test");
    TL_RUN_TEST(test_mississippi);
    TL_RUN_TEST(test_random);
    TL_END_SUITE();
}
