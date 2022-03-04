#include "testlib.h"
#include <cstr.h>

static TL_TEST(creating_bit_vectors)
{
    TL_BEGIN();

    cstr_bit_vector *bv = cstr_new_bv_init(64);
    TL_ERROR_IF_NEQ_LL(bv->no_words, 1ll);
    TL_ERROR_IF_NEQ_LL(bv->no_bits, 64ll);
    for (long long i = 0; i < bv->no_bits; i++)
    {
        TL_ERROR_IF(cstr_bv_get(bv, i));
    }
    free(bv);

    bv = cstr_new_bv_init(63);
    TL_ERROR_IF_NEQ_LL(bv->no_words, 1ll);
    TL_ERROR_IF_NEQ_LL(bv->no_bits, 63ll);
    for (long long i = 0; i < bv->no_bits; i++)
    {
        TL_ERROR_IF(cstr_bv_get(bv, i));
    }
    free(bv);

    bv = cstr_new_bv_init(65);
    TL_ERROR_IF_NEQ_LL(bv->no_words, 2ll);
    TL_ERROR_IF_NEQ_LL(bv->no_bits, 65ll);
    for (long long i = 0; i < bv->no_bits; i++)
    {
        TL_FATAL_IF(cstr_bv_get(bv, i));
    }
    free(bv);

    bv = cstr_new_bv_from_string("001001");
    TL_ERROR_IF_NEQ_INT(0, cstr_bv_get(bv, 0));
    TL_ERROR_IF_NEQ_INT(0, cstr_bv_get(bv, 1));
    TL_ERROR_IF_NEQ_INT(1, cstr_bv_get(bv, 2));
    TL_ERROR_IF_NEQ_INT(0, cstr_bv_get(bv, 3));
    TL_ERROR_IF_NEQ_INT(0, cstr_bv_get(bv, 4));
    TL_ERROR_IF_NEQ_INT(1, cstr_bv_get(bv, 5));
    free(bv);

    TL_END();
}

static TL_TEST(setting_bits)
{
    TL_BEGIN();

    cstr_bit_vector *bv = cstr_new_bv_init(130);

    // Set even
    for (long long i = 0; i < bv->no_bits; i += 2)
    {
        cstr_bv_set(bv, i, true);
    }
    // Check
    for (long long i = 0; i < bv->no_bits; i++)
    {
        if (i & 1)
        {
            // odd -- shouldn't be set
            TL_ERROR_IF(cstr_bv_get(bv, i));
        }
        else
        {
            // even -- should be set
            TL_ERROR_IF(!cstr_bv_get(bv, i));
        }
    }
    cstr_bv_print(bv);

    // flip bits
    for (long long i = 0; i < bv->no_bits; i++)
    {
        bool old_bit = cstr_bv_get(bv, i);
        cstr_bv_set(bv, i, !cstr_bv_get(bv, i));
        TL_ERROR_IF_EQ_INT(old_bit, cstr_bv_get(bv, i));
    }
    cstr_bv_print(bv);

    // Check
    for (long long i = 0; i < bv->no_bits; i++)
    {
        if (i & 1)
        {
            // odd -- should be set now
            TL_ERROR_IF(!cstr_bv_get(bv, i));
        }
        else
        {
            // even -- should no longer be set
            TL_ERROR_IF(cstr_bv_get(bv, i));
        }
    }

    free(bv);

    TL_END();
}

int main(void)
{
    TL_BEGIN_TEST_SUITE("bit vector test");
    TL_RUN_TEST(creating_bit_vectors);
    TL_RUN_TEST(setting_bits);
    TL_END_SUITE();
}
