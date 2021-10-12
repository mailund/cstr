#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>
#include <testlib.h>

TL_TEST(indexing)
{
    TL_BEGIN();
    
    cstr_sslice x = CSTR_SLICE_STRING("foobarbaz");
    
    char fwrd[] = "foobarbaz";
    char bwrd[] = "zabraboof";
    
    for (int i = 0; i < x.len; i++) {
        TL_ERROR_IF_NEQ("%c", CSTR_IDX(x, i), fwrd[i]);
    }

    for (int i = 0; i < x.len; i++) {
        TL_ERROR_IF_NEQ("%c", CSTR_IDX(x, -i - 1), bwrd[i]);
    }

    TL_END();
}

TL_TEST(slices)
{
    TL_BEGIN();
    cstr_sslice x = CSTR_SLICE_STRING("foobarbaz");
    TL_FATAL_IF(x.len != strlen("foobarbaz"));

    TL_ERROR_IF(
        !cstr_sslice_eq(CSTR_SLICE_STRING("bar"),
                        CSTR_SUBSLICE(x, 3, 6)));

    TL_ERROR_IF(
                !cstr_sslice_eq(CSTR_SLICE_STRING("bar"),
                                CSTR_SUBSLICE(x, -6, -3)));

    TL_ERROR_IF(
        !cstr_sslice_eq(CSTR_SLICE_STRING("foo"),
                        CSTR_PREFIX(x, 3)));
    TL_ERROR_IF(
                !cstr_sslice_eq(CSTR_SLICE_STRING("foobar"),
                                CSTR_PREFIX(x, -3)));

    TL_ERROR_IF(
        !cstr_sslice_eq(CSTR_SLICE_STRING("barbaz"),
                        CSTR_SUFFIX(x, 3)));
    TL_ERROR_IF(
                !cstr_sslice_eq(CSTR_SLICE_STRING("baz"),
                                CSTR_SUFFIX(x, -3)));

    TL_END();
}

int main(void)
{
    TL_BEGIN_TEST_SUITE("cstr");
    TL_RUN_TEST(indexing);
    TL_RUN_TEST(slices);
    TL_END_SUITE();
}
