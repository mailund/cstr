#include "testlib.h"
#include "unittests.h"
#include <cstr.h>
#include <limits.h>

// clang-format off
// We will use the largest unsigned int to mean undefined
// static const unsigned int UNDEF = UINT_MAX;
// static inline bool is_undef(unsigned int val) { return val == UNDEF; }
// static inline bool is_def(unsigned int val)   { return !is_undef(val); }

#define IS_S(I)   cstr_bv_get(is_s, I)
#define IS_LMS(I) is_lms(is_s, I)
//static inline bool is_lms(cstr_bit_vector *is_s, long long i)
//{ return (i != 0) && IS_S(i) && !IS_S(i - 1); }

// clang-format on

// MARK: Bucket sort stuff
static inline long long *alloc_buckets(cstr_alphabet *alpha)
{
    long long *buckets = cstr_malloc(alpha->size * sizeof *buckets);
    return buckets;
}

static void count_buckets(cstr_uislice x, long long sigma, long long buckets[sigma])
{
    for (long long i = 0; i < sigma; i++)
    {
        buckets[i] = 0;
    }
    for (long long i = 0; i < x.len; i++)
    {
        buckets[x.buf[i]]++;
    }
}

static void init_buckets_start(long long sigma, long long start[sigma], long long buckets[sigma])
{
    long long sum = 0;
    for (long long i = 0; i < sigma; i++)
    {
        start[i] = sum;
        sum += buckets[i];
    }
}

static void init_buckets_end(long long sigma, long long end[sigma], long long buckets[sigma])
{
    long long sum = 0;
    for (long long i = 0; i < sigma; i++)
    {
        sum += buckets[i];
        end[i] = sum;
    }
}

static void classify_sl(cstr_uislice x, cstr_bit_vector *is_s)
{
    if (x.len == 0)
    {
        return;
    }

    cstr_bv_set(is_s, x.len - 1, true);
    for (long long i = x.len - 1; i > 0; i--)
    {
        bool smaller_start = (x.buf[i - 1] < x.buf[i]);
        bool equal_class = ((x.buf[i - 1] == x.buf[i]) && cstr_bv_get(is_s, i));
        cstr_bv_set(is_s, i - 1, smaller_start || equal_class);
    }
}

#if 0
static inline void undefine_sa_slice(cstr_suffix_array sa)
{
    for (long long i = 0; i < sa.len; i++)
        sa.buf[i] = UNDEF;
}
#endif

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

TL_TEST(buckets_mississippi)
{
    TL_BEGIN();

    // Setup
    cstr_const_sslice u = CSTR_SLICE_STRING0((const char *)"mississippi");
    cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, u);

    cstr_uislice x = CSTR_ALLOC_SLICE_BUFFER(x, u.len);
    bool ok = cstr_alphabet_map_to_uint(x, u, &alpha);
    TL_FATAL_IF(!ok);

    // Tests...
    long long *buckets = alloc_buckets(&alpha);
    long long *buck_ptr = alloc_buckets(&alpha);

    count_buckets(x, alpha.size, buckets);
    TL_FATAL_IF_NEQ_LL(buckets[0], 1LL); // $
    TL_FATAL_IF_NEQ_LL(buckets[1], 4LL); // i
    TL_FATAL_IF_NEQ_LL(buckets[2], 1LL); // m
    TL_FATAL_IF_NEQ_LL(buckets[3], 2LL); // p
    TL_FATAL_IF_NEQ_LL(buckets[4], 4LL); // s

    init_buckets_start(alpha.size, buck_ptr, buckets);
    TL_FATAL_IF_NEQ_LL(buck_ptr[0], 0LL); // $
    TL_FATAL_IF_NEQ_LL(buck_ptr[1], 1LL); // i
    TL_FATAL_IF_NEQ_LL(buck_ptr[2], 5LL); // m
    TL_FATAL_IF_NEQ_LL(buck_ptr[3], 6LL); // p
    TL_FATAL_IF_NEQ_LL(buck_ptr[4], 8LL); // s

    init_buckets_end(alpha.size, buck_ptr, buckets);
    TL_FATAL_IF_NEQ_LL(buck_ptr[0], 1LL);  // $
    TL_FATAL_IF_NEQ_LL(buck_ptr[1], 5LL);  // i
    TL_FATAL_IF_NEQ_LL(buck_ptr[2], 6LL);  // m
    TL_FATAL_IF_NEQ_LL(buck_ptr[3], 8LL);  // p
    TL_FATAL_IF_NEQ_LL(buck_ptr[4], 12LL); // s

    // Cleanup
    free(buckets);
    free(buck_ptr);
    CSTR_FREE_SLICE_BUFFER(x);

    TL_END();
}

TL_TEST(sais_classify_sl_mississippi)
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
    CSTR_FREE_SLICE_BUFFER(x);

    TL_END();
}

TL_TEST(sais_classify_sl_random)
{
    TL_BEGIN();

    // Setup
    const long long n = 10;

    cstr_const_sslice letters = CSTR_SLICE_STRING0((const char *)"acgt");
    cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, letters);

    cstr_sslice x = CSTR_ALLOC_SLICE_BUFFER(x, n);
    cstr_uislice u = CSTR_ALLOC_SLICE_BUFFER(u, n);

    assert(x.buf); // For the static analyser
    assert(u.buf); // For the static analyser

    cstr_bit_vector *is_s = cstr_new_bv(u.len);

    // Tests...
    classify_sl(u, is_s);
    cstr_bv_print(is_s);

    for (int k = 0; k < 10; k++)
    {
        // len-1 since we don't want to sample the sentinel
        tl_random_string0(x, letters.buf, (int)letters.len - 1);
        bool ok = cstr_alphabet_map_to_uint(u, CSTR_SLICE_CONST_CAST(x), &alpha);
        TL_FATAL_IF(!ok);

        classify_sl(u, is_s);
        for (long long i = 0; i < is_s->no_bits - 1; i++)
        {
            if (IS_S(i))
            {
                TL_FATAL_IF_GE_SLICE(CSTR_SUFFIX(u, i), CSTR_SUFFIX(u, i + 1));
            }
            else
            {
                TL_FATAL_IF_LE_SLICE(CSTR_SUFFIX(u, i), CSTR_SUFFIX(u, i + 1));
            }
        }
        TL_FATAL_IF_NEQ_INT(IS_S(n - 1), 1);
    }

    CSTR_FREE_SLICE_BUFFER(x);
    CSTR_FREE_SLICE_BUFFER(u);
    free(is_s);

    TL_END();
}

#endif
