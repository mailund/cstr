#include "cstr.h"

struct cstr_bit_vector *cstr_new_bv(size_t no_bits)
{
    size_t no_words = (no_bits + 63) / 64; // divide by word size and round up
    struct cstr_bit_vector *bv = CSTR_MALLOC_FLEX_ARRAY(bv, words, no_words);
    bv->no_bits = no_bits;
    bv->no_words = no_words;
    for (size_t i = 0; i < no_words; i++)
    {
        bv->words[i] = 0;
    }
    return bv;
}


void cstr_bv_fprint(FILE *f, cstr_bit_vector *bv)
{
    fprintf(f, "[");
    for (size_t i = 0; i < bv->no_bits; i++) {
        if (i > 0 && i % 64 == 0) fprintf(f, "|");
        fprintf(f, "%d", cstr_bv_get(bv, i));
    }
    fprintf(f, "]\n");
}
