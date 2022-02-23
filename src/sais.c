#include "testlib.h"
#include "unittests.h"
#include <cstr.h>
#include <limits.h>

// We will use the largest unsigned int to mean undefined
#define UNDEF UINT_MAX
#define IS_UNDEF(X) ((X) == UNDEF)

#define IS_S(I) (cstr_bv_get(is_s, I))

static void classify_sl(cstr_uislice x, cstr_bit_vector *is_s)
{
    if (x.len == 0)
        return;

    cstr_bv_set(is_s, x.len - 1, true);
    for (long long i = x.len - 1; i > 0; i--)
    {
        bool smaller_start = (x.buf[i - 1] < x.buf[i]);
        bool equal_class = ((x.buf[i - 1] == x.buf[i]) && cstr_bv_get(is_s, i));
        cstr_bv_set(is_s, i - 1, smaller_start || equal_class);
    }
}

//static inline void undefine_sa_slice(cstr_suffix_array sa)
//{
//    for (size_t i = 0; i < sa.len; i++)
//        sa.buf[i] = UNDEF;
//}

static void sais_rec(cstr_suffix_array sa, cstr_const_uislice x,
                     cstr_bit_vector *is_s, unsigned int alph_size)
{
    if (alph_size == x.len)
    {
        // We are done with recursing when all letters are unique.
        // We just need to sort them in their buckets.
        for (unsigned int i = 0; i < x.len; i++)
        {
            sa.buf[x.buf[i]] = i;
        }
        return;
    }
}

void cstr_sais(cstr_suffix_array sa, cstr_const_uislice x, cstr_alphabet *alpha)
{
    cstr_bit_vector *is_s = cstr_new_bv(x.len);
    sais_rec(sa, x, is_s, alpha->size);
    free(is_s);
}

#ifdef GEN_UNIT_TESTS // unit testing of static functions...


TL_TEST(sais_classify_sl)
{
    TL_BEGIN();

    // Setup
    cstr_const_sslice u = CSTR_SLICE_STRING0((const char *)"mississippi");
    cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, u);

    cstr_uislice x = CSTR_ALLOC_SLICE_BUFFER(x, u.len);
    bool ok = cstr_alphabet_map_to_uint(x, u, &alpha);
    TL_FATAL_IF(!ok);

    cstr_bit_vector *is_s = cstr_new_bv(x.len);

    // Tests...
    classify_sl(x, is_s);
    cstr_bv_print(is_s);

    // -S--S--S---S
    // mississippi$
    // 010010010001
    cstr_bit_vector *expected = cstr_new_bv_from_string("010010010001");
    TL_FATAL_IF(!cstr_bv_eq(is_s, expected));
    free(expected);

    free(is_s);

    TL_END();
}

#endif
