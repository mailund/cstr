#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>

#include "testlib.h"

TL_TEST(test_create_alphabet)
{
    TL_BEGIN();

    cstr_alphabet alpha;
    cstr_const_sslice x = CSTR_SLICE_STRING0((const char *)"foobar");
    TL_FATAL_IF_NEQ_LL(x.len, (long long)strlen("foobar") + 1);
    TL_FATAL_IF_NEQ_STRING(x.buf, "foobar");

    cstr_init_alphabet(&alpha, x);

    TL_ERROR_IF_NEQ_INT(alpha.map[0], 0);
    TL_ERROR_IF_NEQ_INT(alpha.map['a'], 1);
    TL_ERROR_IF_NEQ_INT(alpha.map['b'], 2);
    TL_ERROR_IF_NEQ_INT(alpha.map['f'], 3);
    TL_ERROR_IF_NEQ_INT(alpha.map['o'], 4);
    TL_ERROR_IF_NEQ_INT(alpha.map['r'], 5);
    TL_ERROR_IF_NEQ_INT(6, alpha.size);

    TL_ERROR_IF_NEQ_INT(alpha.revmap[1], 'a');

    for (int i = 0; i < 256; i++)
    {
        // If we have a map, it is in error if it doesn't rev back.
        TL_ERROR_IF(!(alpha.map[i] & ~0xff) && alpha.revmap[alpha.map[i]] != i);
    }

    TL_END();
}

TL_TEST(test_mapping)
{
    TL_BEGIN();

    bool ok = true;

    cstr_const_sslice x = CSTR_SLICE_STRING0((const char *)"foobar");
    TL_ERROR_IF_NEQ_LL(x.len, 7ll);

    cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, x);

    cstr_sslice mapped = CSTR_ALLOC_SLICE_BUFFER(mapped, x.len);
    TL_ERROR_IF_NEQ_LL(x.len, mapped.len);

    ok = cstr_alphabet_map(mapped, x, &alpha);
    TL_FATAL_IF(!ok);

    cstr_sslice expected = CSTR_SLICE_STRING0("\3\4\4\2\1\5");
    TL_ERROR_IF_NEQ_SLICE(mapped, expected);
    CSTR_FREE_SLICE_BUFFER(mapped);

    mapped = CSTR_ALLOC_SLICE_BUFFER(mapped, 3);
    ok = cstr_alphabet_map(mapped, CSTR_SLICE_STRING((const char *)"qux"), &alpha);
    TL_ERROR_IF(ok);

    CSTR_FREE_SLICE_BUFFER(mapped);

    TL_END();
}

TL_TEST(test_int_mapping)
{
    TL_BEGIN();

    cstr_alphabet alpha;
    cstr_const_sslice x = CSTR_SLICE_STRING0((const char *)"foobar");
    cstr_init_alphabet(&alpha, x);

    cstr_uislice mapped = CSTR_ALLOC_SLICE_BUFFER(mapped, x.len);
    bool ok = cstr_alphabet_map_to_uint(mapped, x, &alpha);
    TL_FATAL_IF(!ok);

    int expected[] = {3, 4, 4, 2, 1, 5, 0};
    TL_TEST_EQUAL_INT_ARRAYS(expected, mapped.buf,
                             sizeof(expected) / sizeof(*expected));
    CSTR_FREE_SLICE_BUFFER(mapped);

    mapped = CSTR_ALLOC_SLICE_BUFFER(mapped, 4);
    ok = cstr_alphabet_map_to_uint(mapped, CSTR_SLICE_STRING0((const char *)"qux"), &alpha);
    TL_ERROR_IF(ok);

    CSTR_FREE_SLICE_BUFFER(mapped);

    TL_END();
}

TL_TEST(test_revmapping)
{
    TL_BEGIN();

    cstr_alphabet alpha;
    cstr_const_sslice x = CSTR_SLICE_STRING0((const char *)"foobar");
    cstr_init_alphabet(&alpha, x);

    cstr_sslice mapped = CSTR_ALLOC_SLICE_BUFFER(mapped, x.len);

    bool ok = cstr_alphabet_map(mapped, x, &alpha);
    TL_FATAL_IF(!ok);

    cstr_sslice rev = CSTR_ALLOC_SLICE_BUFFER(rev, x.len);
    ok = cstr_alphabet_revmap(rev, CSTR_SLICE_CONST_CAST(mapped), &alpha);
    TL_FATAL_IF(!ok);

    TL_ERROR_IF_NEQ_SLICE(CSTR_SLICE_CONST_CAST(x), rev);

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
