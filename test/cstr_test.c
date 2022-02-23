#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>
#include <testlib.h>

static TL_TEST(indexing)
{
    TL_BEGIN();

    cstr_sslice x = CSTR_SLICE_STRING("foobarbaz");

    char fwrd[] = "foobarbaz";
    char bwrd[] = "zabraboof";

    for (int i = 0; i < x.len; i++)
    {
        TL_ERROR_IF_NEQ("%c", CSTR_IDX(x, i), fwrd[i]);
    }

    for (int i = 0; i < x.len; i++)
    {
        TL_ERROR_IF_NEQ("%c", CSTR_IDX(x, -i - 1), bwrd[i]);
    }

    TL_END();
}

static TL_TEST(slices)
{
    TL_BEGIN();
    cstr_sslice x = CSTR_SLICE_STRING("foobarbaz");
    TL_FATAL_IF(x.len != strlen("foobarbaz")); // Flawfinder: ignore

    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("bar"),
                        CSTR_SUBSLICE(x, 3, 6)));

    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("bar"),
                        CSTR_SUBSLICE(x, -6, -3)));

    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("foo"),
                        CSTR_PREFIX(x, 3)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("foobar"),
                        CSTR_PREFIX(x, -3)));

    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("barbaz"),
                        CSTR_SUFFIX(x, 3)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("baz"),
                        CSTR_SUFFIX(x, -3)));

    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING(""),
                        CSTR_PREFIX(x, 0)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("f"),
                        CSTR_PREFIX(x, 1)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("fo"),
                        CSTR_PREFIX(x, 2)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("foo"),
                        CSTR_PREFIX(x, 3)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("foob"),
                        CSTR_PREFIX(x, 4)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("fooba"),
                        CSTR_PREFIX(x, 5)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("foobar"),
                        CSTR_PREFIX(x, 6)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("foobarb"),
                        CSTR_PREFIX(x, 7)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("foobarba"),
                        CSTR_PREFIX(x, 8)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("foobarbaz"),
                        CSTR_PREFIX(x, 9)));

    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("foobarba"),
                        CSTR_PREFIX(x, -1)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("foobarb"),
                        CSTR_PREFIX(x, -2)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("foobar"),
                        CSTR_PREFIX(x, -3)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("fooba"),
                        CSTR_PREFIX(x, -4)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("foob"),
                        CSTR_PREFIX(x, -5)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("foo"),
                        CSTR_PREFIX(x, -6)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("fo"),
                        CSTR_PREFIX(x, -7)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("f"),
                        CSTR_PREFIX(x, -8)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING(""),
                        CSTR_PREFIX(x, -9)));

    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING(""),
                        CSTR_SUFFIX(x, x.len)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("z"),
                        CSTR_SUFFIX(x, x.len - 1)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("az"),
                        CSTR_SUFFIX(x, x.len - 2)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("baz"),
                        CSTR_SUFFIX(x, x.len - 3)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("rbaz"),
                        CSTR_SUFFIX(x, x.len - 4)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("arbaz"),
                        CSTR_SUFFIX(x, x.len - 5)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("barbaz"),
                        CSTR_SUFFIX(x, x.len - 6)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("obarbaz"),
                        CSTR_SUFFIX(x, x.len - 7)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("oobarbaz"),
                        CSTR_SUFFIX(x, x.len - 8)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("foobarbaz"),
                        CSTR_SUFFIX(x, x.len - 9)));

    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("z"),
                        CSTR_SUFFIX(x, -1)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("az"),
                        CSTR_SUFFIX(x, -2)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("baz"),
                        CSTR_SUFFIX(x, -3)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("rbaz"),
                        CSTR_SUFFIX(x, -4)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("arbaz"),
                        CSTR_SUFFIX(x, -5)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("barbaz"),
                        CSTR_SUFFIX(x, -6)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("obarbaz"),
                        CSTR_SUFFIX(x, -7)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("oobarbaz"),
                        CSTR_SUFFIX(x, -8)));
    TL_ERROR_IF(
        !cstr_eq_sslice(CSTR_SLICE_STRING("foobarbaz"),
                        CSTR_SUFFIX(x, -9)));

    TL_END();
}

static TL_TEST(eq)
{
    TL_BEGIN();

    cstr_sslice x = CSTR_SLICE_STRING("foobar");
    cstr_sslice y = CSTR_SLICE_STRING("foobar");
    TL_FATAL_IF(!CSTR_SLICE_EQ(x, y));

    cstr_sslice z = CSTR_SLICE_STRING("foo");
    TL_FATAL_IF(CSTR_SLICE_EQ(x, z));

    TL_END();
}

static TL_TEST(lcp)
{
    TL_BEGIN();

    cstr_sslice x = CSTR_SLICE_STRING("foobar");
    cstr_sslice y = CSTR_SLICE_STRING("foobar");
    TL_FATAL_IF_NEQ_LL(CSTR_SLICE_LCP(x, y), x.len);

    cstr_sslice z = CSTR_SLICE_STRING("foo");
    TL_FATAL_IF_NEQ_LL(CSTR_SLICE_LCP(x, z), z.len);

    cstr_sslice w = CSTR_SLICE_STRING("foobaz");
    TL_FATAL_IF_NEQ_LL(CSTR_SLICE_LCP(x, w), 5ll);

    TL_END();
}

int main(void)
{
    TL_BEGIN_TEST_SUITE("cstr");
    TL_RUN_TEST(indexing);
    TL_RUN_TEST(slices);
    TL_RUN_TEST(eq);
    TL_RUN_TEST(lcp);
    TL_END_SUITE();
}
