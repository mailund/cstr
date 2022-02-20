#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "testlib.h"
#include <cstr.h>

void check_suffix_ordered(char const *x, cstr_suffix_array sa)
{
    for (int i = 1; i < sa.len; i++)
    {
        printf("%s vs %s\n", x + sa.buf[i - 1], x + sa.buf[i]);
        assert(strcmp(x + sa.buf[i - 1], x + sa.buf[i]) < 0);
    }
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

    for (int i = 0; i < x.len + 1; i++)
    {
        printf("sa[%d] == %d %s\n", i, sa.buf[i], x.buf + sa.buf[i]);
    }

    check_suffix_ordered(x.buf, sa);

    CSTR_FREE_SLICE_BUFFER(sa);

    TL_END();
}

TL_TEST(test_random)
{
    TL_BEGIN();

    const long long n = 100;

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
        tl_random_string(x, letters.buf, letters.len);
        cstr_alphabet_map_to_uint(mapped, x, &alpha);
        cstr_skew(sa, mapped, &alpha);

        for (int i = 0; i < x.len + 1; i++)
        {
            printf("sa[%d] == %d %s\n", i, sa.buf[i], x.buf + sa.buf[i]);
        }

        check_suffix_ordered(x.buf, sa);
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
