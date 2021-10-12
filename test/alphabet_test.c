#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>

#include "testlib.h"

TL_TEST(test_create_alphabet)
{
    TL_BEGIN();

    cstr_alphabet alpha;
    cstr_sslice x = CSTR_SLICE_STRING("foobar");
    cstr_init_alphabet(&alpha, x);

    TL_ERROR_IF(alpha.map[0] != 0);
    TL_ERROR_IF(alpha.map['a'] != 1);
    TL_ERROR_IF(alpha.map['b'] != 2);
    TL_ERROR_IF(alpha.map['f'] != 3);
    TL_ERROR_IF(alpha.map['o'] != 4);
    TL_ERROR_IF(alpha.map['r'] != 5);
    TL_ERROR_IF(6 != alpha.size);

    TL_ERROR_IF(alpha.revmap[1] != 'a');

    for (int i = 0; i < CSTR_NO_CHARS; i++)
    {
        // If we have a map, it is in error if it doesn't rev back.
        TL_ERROR_IF(alpha.map[i] && alpha.revmap[alpha.map[i]] != i);
    }

    TL_END();
}

TL_TEST(test_mapping)
{
    TL_BEGIN();

    bool ok = true;

    cstr_sslice x = CSTR_SLICE_STRING("foobar");
    cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, x);

    cstr_sslice mapped = cstr_alloc_sslice_buffer(x.len);

    ok = cstr_alphabet_map(mapped, x, &alpha);
    TL_FATAL_IF(!ok);

    TL_ERROR_IF(!cstr_sslice_eq(mapped, CSTR_SLICE_STRING("\3\4\4\2\1\5")));
    CSTR_FREE_SLICE_BUFFER(mapped);

    mapped = cstr_alloc_sslice_buffer(3);
    ok = cstr_alphabet_map(mapped, CSTR_SLICE_STRING("qux"), &alpha);
    TL_ERROR_IF(ok);

    CSTR_FREE_SLICE_BUFFER(mapped);

    TL_END();
}

TL_TEST(test_int_mapping)
{
    TL_BEGIN();

    cstr_alphabet alpha;
    cstr_sslice x = CSTR_SLICE_STRING("foobar");
    cstr_init_alphabet(&alpha, x);

    cstr_islice mapped = cstr_alloc_islice_buffer(x.len + 1);
    bool ok = cstr_alphabet_map_to_int(mapped, x, &alpha);
    TL_FATAL_IF(!ok);

    int expected[] = {3, 4, 4, 2, 1, 5, 0};
    TL_TEST_EQUAL_INT_ARRAYS(expected, mapped.buf,
                             sizeof(expected) / sizeof(*expected));
    CSTR_FREE_SLICE_BUFFER(mapped);

    mapped = cstr_alloc_islice_buffer(4);
    ok = cstr_alphabet_map_to_int(mapped, CSTR_SLICE_STRING("qux"), &alpha);
    TL_ERROR_IF(ok);

    CSTR_FREE_SLICE_BUFFER(mapped);

    TL_END();
}

TL_TEST(test_revmapping)
{
    TL_BEGIN();

    cstr_alphabet alpha;
    cstr_sslice x = CSTR_SLICE_STRING("foobar");
    cstr_init_alphabet(&alpha, x);

    cstr_sslice mapped = cstr_alloc_sslice_buffer(x.len);

    bool ok = cstr_alphabet_map(mapped, x, &alpha);
    TL_FATAL_IF(!ok);

    cstr_sslice rev = cstr_alloc_sslice_buffer(x.len);
    ok = cstr_alphabet_revmap(rev, mapped, &alpha);
    TL_FATAL_IF(!ok);

    TL_ERROR_IF(!cstr_sslice_eq(x, rev));

    CSTR_FREE_SLICE_BUFFER(mapped);
    CSTR_FREE_SLICE_BUFFER(rev);

    TL_END();
}

int main(void)
{
    TL_BEGIN_TEST_SUITE("alphabet_test");
    TL_RUN_TEST(test_create_alphabet);
    TL_RUN_TEST(test_mapping);
    TL_RUN_TEST(test_int_mapping);
    TL_RUN_TEST(test_revmapping);
    TL_END_SUITE();
}
