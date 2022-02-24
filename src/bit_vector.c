#include "cstr.h"

void cstr_bv_clear(cstr_bit_vector *bv)
{
    for (long long i = 0; i < bv->no_words; i++)
    {
        bv->words[i] = 0ULL;
    }
}

bool cstr_bv_eq(cstr_bit_vector *a, cstr_bit_vector *b)
{
    if (a->no_bits != b->no_bits)
    {
        return false;
    }

    for (long long i = 0; i < a->no_words; i++)
    {
        if (a->words[i] != b->words[i])
        {
            return false;
        }
    }
    return true;
}

void cstr_bv_fprint(FILE *f, cstr_bit_vector *bv)
{
    const long long bits_per_word = 64;
    fprintf(f, "[");
    for (long long i = 0; i < bv->no_bits; i++)
    {
        if (i > 0 && i % bits_per_word == 0)
        {
            fprintf(f, "|");
        }
        fprintf(f, "%d", cstr_bv_get(bv, i));
    }
    fprintf(f, "]\n");
}
