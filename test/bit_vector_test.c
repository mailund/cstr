#include <cstr.h>
#include "testlib.h"

TL_TEST(creating_bit_vectors)
{
    TL_BEGIN();

    cstr_bit_vector *bv = cstr_new_bv(64);
    TL_ERROR_IF_NEQ_SIZE_T(bv->no_words, 1ul);
    TL_ERROR_IF_NEQ_SIZE_T(bv->no_bits, 64ul);
    for (size_t i = 0; i < bv->no_bits; i++) {
        TL_ERROR_IF(cstr_bv_get(bv, i));
    }
    free(bv);

    bv = cstr_new_bv(63);
    TL_ERROR_IF_NEQ_SIZE_T(bv->no_words, 1ul);
    TL_ERROR_IF_NEQ_SIZE_T(bv->no_bits, 63ul);
    for (size_t i = 0; i < bv->no_bits; i++) {
        TL_ERROR_IF(cstr_bv_get(bv, i));
    }
    free(bv);

    bv = cstr_new_bv(65);
    TL_ERROR_IF_NEQ_SIZE_T(bv->no_words, 2ul);
    TL_ERROR_IF_NEQ_SIZE_T(bv->no_bits, 65ul);
    for (size_t i = 0; i < bv->no_bits; i++) {
        TL_ERROR_IF(cstr_bv_get(bv, i));
    }
    free(bv);

    TL_END();
}

int main(void)
{
    TL_BEGIN_TEST_SUITE("bit vector test");
    TL_RUN_TEST(creating_bit_vectors);
    TL_END_SUITE();
}
