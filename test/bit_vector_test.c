#include "testlib.h"
#include <cstr.h>

TL_TEST(creating_bit_vectors)
{
    TL_BEGIN();

    cstr_bit_vector *bv = cstr_new_bv(64);
    TL_FATAL_IF_NEQ_SIZE_T(bv->no_words, 1ul);
    TL_FATAL_IF_NEQ_SIZE_T(bv->no_bits, 64ul);
    for (size_t i = 0; i < bv->no_bits; i++)
    {
        TL_FATAL_IF(cstr_bv_get(bv, i));
    }
    free(bv);

    bv = cstr_new_bv(63);
    TL_FATAL_IF_NEQ_SIZE_T(bv->no_words, 1ul);
    TL_FATAL_IF_NEQ_SIZE_T(bv->no_bits, 63ul);
    for (size_t i = 0; i < bv->no_bits; i++)
    {
        TL_FATAL_IF(cstr_bv_get(bv, i));
    }
    free(bv);

    bv = cstr_new_bv(65);
    TL_FATAL_IF_NEQ_SIZE_T(bv->no_words, 2ul);
    TL_FATAL_IF_NEQ_SIZE_T(bv->no_bits, 65ul);
    for (size_t i = 0; i < bv->no_bits; i++)
    {
        TL_FATAL_IF(cstr_bv_get(bv, i));
    }
    free(bv);

    TL_END();
}

TL_TEST(setting_bits)
{
    TL_BEGIN();

    cstr_bit_vector *bv = cstr_new_bv(130);

    // Set even
    for (size_t i = 0; i < bv->no_bits; i += 2)
    {
        cstr_bv_set(bv, i, true);
    }
    // Check
    for (size_t i = 0; i < bv->no_bits; i++)
    {
        if (i & 1)
        {
            // odd -- shouldn't be set
            TL_FATAL_IF(cstr_bv_get(bv, i));
        }
        else
        {
            // even -- should be set
            TL_FATAL_IF(!cstr_bv_get(bv, i));
        }
    }
    cstr_bv_print(bv);

    // flip bits
    for (size_t i = 0; i < bv->no_bits; i++)
    {
        bool old_bit = cstr_bv_get(bv, i);
        cstr_bv_set(bv, i, !cstr_bv_get(bv, i));
        TL_FATAL_IF_EQ_INT(old_bit, cstr_bv_get(bv, i));
    }
    cstr_bv_print(bv);
    
    // Check
    for (size_t i = 0; i < bv->no_bits; i++)
    {
        if (i & 1)
        {
            // odd -- should be set now
            TL_FATAL_IF(!cstr_bv_get(bv, i));
        }
        else
        {
            // even -- should no longer be set
            TL_FATAL_IF(cstr_bv_get(bv, i));
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
