#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>

#include "testlib.h"

TL_TEST(test_create_alphabet)
{
    TL_BEGIN();

    char const *x = "foobar";
    struct cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, CSTR_CSSLICE_STRING(x));

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

    enum cstr_errcodes err;

    char const *x = "foobar";
    struct cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, CSTR_CSSLICE_STRING(x));

    char *mapped = cstr_alphabet_map_new(CSTR_CSSLICE_STRING(x), &alpha, &err);
    TL_FATAL_IF(mapped == NULL);
    TL_FATAL_IF(CSTR_NO_ERROR != err);

    TL_ERROR_IF(strcmp(mapped, "\3\4\4\2\1\5") != 0);

    free(mapped);
    mapped = 0;

    mapped = cstr_alphabet_map_new(CSTR_CSSLICE_STRING("qux"), &alpha, &err);
    TL_ERROR_IF(mapped != NULL);
    TL_ERROR_IF(CSTR_MAPPING_ERROR != err);

    // let us see if we get an error if the dst has the wrong length.
    // we use a null pointer, but it is okay because we shouldn't even touch it
    bool ok = cstr_alphabet_map(CSTR_SSLICE(0, 3), CSTR_CSSLICE_STRING(x), &alpha, &err);
    TL_ERROR_IF(ok);
    TL_ERROR_IF(CSTR_SIZE_ERROR != err);

    ok = cstr_alphabet_map(CSTR_SSLICE(0, 30), CSTR_CSSLICE_STRING(x), &alpha, &err);
    TL_ERROR_IF(ok);
    TL_ERROR_IF(CSTR_SIZE_ERROR != err);

    TL_END();
}

TL_TEST(test_int_mapping)
{
    TL_BEGIN();

    enum cstr_errcodes err;

    char const *x = "foobar";
    struct cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, CSTR_CSSLICE_STRING(x));

    unsigned int *mapped = cstr_alphabet_map_to_int_new(CSTR_CSSLICE_STRING(x), &alpha, &err);
    TL_FATAL_IF(mapped == NULL);
    TL_FATAL_IF(CSTR_NO_ERROR != err);

    int expected[] = {3, 4, 4, 2, 1, 5, 0};
    TL_TEST_EQUAL_INT_ARRAYS(expected, mapped,
                             sizeof(expected) / sizeof(*expected));
    free(mapped);
    mapped = 0;

    mapped = cstr_alphabet_map_to_int_new(CSTR_CSSLICE_STRING("qux"), &alpha, &err);
    TL_ERROR_IF(mapped != NULL);
    TL_ERROR_IF(CSTR_MAPPING_ERROR != err);

    TL_END();
}

TL_TEST(test_revmapping)
{
    TL_BEGIN();

    enum cstr_errcodes err;

    char const *x = "foobar";
    struct cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, CSTR_CSSLICE_STRING(x));

    char *mapped = cstr_alphabet_map_new(CSTR_CSSLICE_STRING(x), &alpha, &err);
    char *rev = cstr_alphabet_revmap_new(CSTR_CSSLICE_STRING(mapped), &alpha, &err);

    TL_FATAL_IF(CSTR_NO_ERROR != err);
    TL_ERROR_IF(strcmp(x, rev) != 0);

    free(rev);
    free(mapped);

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
