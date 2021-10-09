#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>
#include <cstr_internal.h>

#include "testlib.h"

TL_TEST(test_create_alphabet)
{
    TL_BEGIN();

    struct cstr_alphabet alpha;
    sslice x = CSTR_SSLICE_STRING("foobar");
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
    enum cstr_errcodes err;

    
    sslice x = CSTR_SSLICE_STRING("foobar");
    struct cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, x);
    
    sslice mapped = CSTR_ALLOC_SSLICE(x.len);
    
    ok = cstr_alphabet_map(mapped, x, &alpha, &err);
    TL_FATAL_IF(!ok);
    TL_FATAL_IF(CSTR_NO_ERROR != err);
    

    TL_ERROR_IF(!cstr_sslice_eq(mapped, CSTR_SSLICE_STRING("\3\4\4\2\1\5")));
    CSTR_FREE_SLICE_BUFFER(mapped);
    
    mapped = CSTR_ALLOC_SSLICE(3);
    ok = cstr_alphabet_map(mapped, CSTR_SSLICE_STRING("qux"), &alpha, &err);
    TL_ERROR_IF(ok);
    TL_ERROR_IF(CSTR_MAPPING_ERROR != err);

    CSTR_FREE_SLICE_BUFFER(mapped);
        
    TL_END();
}

TL_TEST(test_int_mapping)
{
    TL_BEGIN();

    enum cstr_errcodes err;

    struct cstr_alphabet alpha;
    sslice x = CSTR_SSLICE_STRING("foobar");
    cstr_init_alphabet(&alpha, x);

    islice mapped = CSTR_ALLOC_ISLICE(x.len + 1);
    bool ok = cstr_alphabet_map_to_int(mapped, x, &alpha, &err);
    TL_FATAL_IF(!ok);
    TL_FATAL_IF(CSTR_NO_ERROR != err);

    int expected[] = {3, 4, 4, 2, 1, 5, 0};
    TL_TEST_EQUAL_INT_ARRAYS(expected, mapped.buf,
                             sizeof(expected) / sizeof(*expected));
    CSTR_FREE_SLICE_BUFFER(mapped);
    
    mapped = CSTR_ALLOC_ISLICE(4);
    ok = cstr_alphabet_map_to_int(mapped, CSTR_SSLICE_STRING("qux"), &alpha, &err);
    TL_ERROR_IF(ok);
    TL_ERROR_IF(CSTR_MAPPING_ERROR != err);
    
    CSTR_FREE_SLICE_BUFFER(mapped);

    TL_END();
}

TL_TEST(test_revmapping)
{
    TL_BEGIN();

    enum cstr_errcodes err;

    struct cstr_alphabet alpha;
    sslice x = CSTR_SSLICE_STRING("foobar");
    cstr_init_alphabet(&alpha, x);

    sslice mapped = CSTR_ALLOC_SSLICE(x.len);
    
    bool ok = cstr_alphabet_map(mapped, x, &alpha, &err);
    TL_FATAL_IF(!ok);
    TL_FATAL_IF(CSTR_NO_ERROR != err);

    sslice rev = CSTR_ALLOC_SSLICE(x.len);
    ok = cstr_alphabet_revmap(rev, mapped, &alpha, &err);
    TL_FATAL_IF(!ok);
    TL_FATAL_IF(CSTR_NO_ERROR != err);
    
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
