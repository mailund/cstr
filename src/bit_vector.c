#include "cstr.h"

void cstr_bv_clear(cstr_bit_vector *bv)
{
    for (long long i = 0; i < bv->no_words; i++)
    {
        bv->words[i] = 0ULL;
    }
}

cstr_bit_vector *cstr_new_bv(long long no_bits)
{
    const long long bits_per_word = 64;
    long long no_words = (no_bits + bits_per_word - 1) / bits_per_word; // divide by word size and round up
    cstr_bit_vector *bv = CSTR_MALLOC_FLEX_ARRAY(bv, words, (size_t)no_words);
    bv->no_bits = no_bits;
    bv->no_words = no_words;
    // Clearing the last word, so we know the left-over bits are zero, makes
    // it faster to compare whole bit vectors.
    bv->words[bv->no_words - 1] = (uint64_t)0;
    return bv;
}

cstr_bit_vector *cstr_new_bv_init(long long no_bits)
{
    cstr_bit_vector *bv = cstr_new_bv(no_bits);
    cstr_bv_clear(bv);
    return bv;
}

cstr_bit_vector *cstr_new_bv_from_string(const char *bits)
{
    const long long no_bits = cstr_strlen(bits);
    cstr_bit_vector *bv = cstr_new_bv(no_bits);
    for (long long i = 0; i < no_bits; i++)
    {
        cstr_bv_set(bv, i, bits[i] == '1');
    }
    return bv;
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
