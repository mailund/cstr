#include "testlib.h"
#include "unittests.h"
#include <cstr.h>
#include <limits.h>

// clang-format off
// We will use the largest unsigned int to mean undefined
static const unsigned int UNDEF = UINT_MAX;
static inline bool is_undef(unsigned int val) { return val == UNDEF; }
static inline bool is_def(unsigned int val)   { return !is_undef(val); }

#define IS_S(I)   cstr_bv_get(is_s, I)
#define IS_L(I)   (!IS_S(I))
#define IS_LMS(I) is_lms(is_s, I)
static inline bool is_lms(cstr_bit_vector *is_s, long long i)
{ return (i != 0) && IS_S(i) && !IS_S(i - 1); }

// clang-format on

// MARK: Bucket sort stuff
__attribute__((malloc)) static inline long long *alloc_buckets(long long sigma)
{
    long long *buckets = cstr_malloc((size_t)sigma * sizeof *buckets);
    return buckets;
}

static void count_buckets(cstr_const_uislice x, long long sigma, long long buckets[sigma])
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

static void init_buckets_start(long long sigma, long long start[sigma],
                               const long long buckets[sigma])
{
    long long sum = 0;
    for (long long i = 0; i < sigma; i++)
    {
        start[i] = sum;
        sum += buckets[i];
    }
}

static void init_buckets_end(long long sigma, long long end[sigma],
                             const long long buckets[sigma])
{
    long long sum = 0;
    for (long long i = 0; i < sigma; i++)
    {
        sum += buckets[i];
        end[i] = sum;
    }
}

static void classify_sl(cstr_const_uislice x, cstr_bit_vector *is_s)
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

static inline void undefine_sa_slice(cstr_suffix_array sa)
{
    for (long long i = 0; i < sa.len; i++)
    {
        sa.buf[i] = UNDEF;
    }
}

static void bucket_lms(cstr_const_uislice x, cstr_suffix_array sa,
                       cstr_bit_vector *is_s, long long ends[])
{
    for (long long i = x.len - 1; i >= 0; i--)
    {
        if (IS_LMS(i))
        {
            sa.buf[--ends[x.buf[i]]] = (unsigned int)i;
        }
    }
}

static void induce_l(cstr_const_uislice x, cstr_suffix_array sa,
                     cstr_bit_vector *is_s, long long start[])
{
    for (long long i = 0; i < x.len; i++)
    {
        if (sa.buf[i] == 0 || is_undef(sa.buf[i]))
        {
            continue;
        }
        long long j = sa.buf[i] - 1;
        if (IS_L(j))
        {
            sa.buf[start[x.buf[j]]++] = (unsigned int)j;
        }
    }
}

static void induce_s(cstr_const_uislice x, cstr_suffix_array sa,
                     cstr_bit_vector *is_s, long long end[])
{
    for (long long i = x.len - 1; i > 0; i--)
    {
        if (sa.buf[i] == 0)
        {
            continue;
        }
        long long j = sa.buf[i] - 1;
        if (IS_S(j))
        {
            sa.buf[--end[x.buf[j]]] = (unsigned int)j;
        }
    }
}

static bool equal_lms_strings(cstr_const_uislice x, cstr_bit_vector *is_s,
                              long long i, long long j)
{
    // They are obviously equal if they are the same string...
    if (i == j)
    {
        return true;
    }

    // Now they can't be equal, so if one is the sentinel, they are different
    if (i == x.len - 1 || j == x.len - 1)
    {
        return false;
    }

    // Now we can scan along until we see a difference or reach the next LMS index
    for (long long k = 0;; k++)
    {
        // If we reach the end of both strings, they are equal.
        // The k > 0 to not test at the very first index where both
        // are obviously also LMS.
        if (k > 0 && IS_LMS(i + k) && IS_LMS(j + k))
        {
            return true;
        }
        if (IS_LMS(i + k) != IS_LMS(j + k) || x.buf[i + k] != x.buf[j + k])
        {
            // We found a difference (in either termination or character)
            return false;
        }
    }

    return false;
}

// Move all the LMS index to the beginning of sa, then put the sub-slice
// that contains them in compact and put the rest of sa in rest.
static cstr_uislice compact_lms(cstr_suffix_array sa,
                                cstr_bit_vector *is_s,
                                cstr_uislice *rest)
{
    long long k = 0;
    for (long long i = 0; i < sa.len; i++)
    {
        long long j = sa.buf[i];
        if (IS_LMS(j))
        {
            sa.buf[k++] = (unsigned int)j;
        }
    }

    *rest = CSTR_SUFFIX(sa, k);
    return CSTR_PREFIX(sa, k);
}

static cstr_uislice compact_defined(cstr_uislice x)
{
    long long k = 0;
    for (long long i = 0; i < x.len; i++)
    {
        if (is_def(x.buf[i]))
        {
            x.buf[k++] = x.buf[i];
        }
    }
    return CSTR_PREFIX(x, k);
}

static cstr_uislice reduce(cstr_const_uislice x, cstr_suffix_array sa, cstr_bit_vector *is_s,
                           cstr_uislice *compact, unsigned int *sigma)
{
    cstr_uislice buffer;
    *compact = compact_lms(sa, is_s, &buffer);
    undefine_sa_slice(buffer);

    // Use buffer to make the map of ordered lms strings, exploiting that
    // we never have two lms index next to each other, so we can map in half
    // the space.
    *sigma = 0;
    long long prev_lms = compact->buf[0];
    buffer.buf[prev_lms / 2] = *sigma;
    for (long long i = 1; i < compact->len; i++)
    {
        unsigned int j = compact->buf[i];
        if (!equal_lms_strings(x, is_s, prev_lms, j))
        {
            (*sigma)++; // We've seen a new letter
        }
        buffer.buf[j / 2] = *sigma;
        prev_lms = j;
    }
    (*sigma)++; // Alphabet size is one larger than the largets letter

    // Now all there is left is to compact the table in buffer into the reduced string
    return compact_defined(buffer);
}

static void reverse_u(cstr_const_uislice x,
                      cstr_suffix_array sa,
                      cstr_bit_vector *is_s,
                      cstr_const_uislice sa_u,
                      cstr_uislice offsets,
                      long long ends[])
{
    // Compact the LMS indices into offset so we have them there
    // in their original order
    long long k = 0;
    for (long long i = 0; i < x.len; i++)
    {
        if (IS_LMS(i))
        {
            offsets.buf[k++] = (unsigned int)i;
        }
    }

    // Now reorder the offsets according to the suffix array of u
    // and put the result at the top of sa
    for (long long i = 0; i < k; i++)
    {
        sa.buf[i] = offsets.buf[sa_u.buf[i]];
    }

    // Data after k isn't used any more, but we need to clear
    // it to undefined for the later imputing.
    undefine_sa_slice(CSTR_SUFFIX(sa, k));

    // Then move the ordered LMS indices to their correct position
    // using bucketing.
    for (long long i = k - 1; i >= 0; i--)
    {
        // Get the next value and undef its entry
        unsigned int j = sa.buf[i];
        sa.buf[i] = UNDEF;
        // Then insert it in the right bucket
        sa.buf[--ends[x.buf[j]]] = j;
    }
}

static void sais_rec(cstr_suffix_array sa, cstr_const_uislice x,
                     cstr_bit_vector *is_s, unsigned int sigma)
{
    if (sigma == x.len)
    {
        // We are done with recursing when all letters are unique.
        // We just need to sort them in their buckets.
        for (unsigned int i = 0; i < x.len; i++)
        {
            sa.buf[x.buf[i]] = i;
        }
        return;
    }

    // Recursive case. We need to sort LMS-strings and create reduced string.
    long long *buckets = alloc_buckets(sigma);
    long long *buck_ptr = alloc_buckets(sigma);
    count_buckets(x, sigma, buckets);
    undefine_sa_slice(sa);
    classify_sl(x, is_s);

    init_buckets_end(sigma, buck_ptr, buckets);
    bucket_lms(x, sa, is_s, buck_ptr);

    init_buckets_start(sigma, buck_ptr, buckets);
    induce_l(x, sa, is_s, buck_ptr);

    init_buckets_end(sigma, buck_ptr, buckets);
    induce_s(x, sa, is_s, buck_ptr);

    CSTR_FREE_NULL(buckets);
    CSTR_FREE_NULL(buck_ptr);

    // Construct u for the recursion
    unsigned int u_sigma;
    cstr_uislice sa_u, u;
    u = reduce(x, sa, is_s, &sa_u, &u_sigma);

    // Now sa_u is the first bit of sa and u the rest of sa. Remember that they overlap.
    // Don't fuck around with sa before you are done with u and sa_u, or things will break.
    // We create u here, but sa_u is just getting working memory, not initialised.

    // Construct suffix array for u
    sais_rec(sa_u, CSTR_SLICE_CONST_CAST(u), is_s, u_sigma);

    // Now we need the LMS strings back from u, in the correct order,
    // and then induce once more.
    buckets = alloc_buckets(sigma);
    buck_ptr = alloc_buckets(sigma);
    count_buckets(x, sigma, buckets);
    classify_sl(x, is_s);

    // Get the sorted LMS strings back into sa and then impute the rest
    init_buckets_end(sigma, buck_ptr, buckets);
    reverse_u(x, sa, is_s, CSTR_SLICE_CONST_CAST(sa_u), u, buck_ptr);

    init_buckets_start(sigma, buck_ptr, buckets);
    induce_l(x, sa, is_s, buck_ptr);

    init_buckets_end(sigma, buck_ptr, buckets);
    induce_s(x, sa, is_s, buck_ptr);

    CSTR_FREE_NULL(buckets);
    CSTR_FREE_NULL(buck_ptr);
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

    cstr_uislice *x_buf = cstr_alloc_uislice(u.len);
    bool ok = cstr_alphabet_map_to_uint(*x_buf, u, &alpha);
    TL_FATAL_IF(!ok);
    cstr_const_uislice x = CSTR_SLICE_CONST_CAST(*x_buf);

    // Tests...
    long long *buckets = alloc_buckets(alpha.size);
    long long *buck_ptr = alloc_buckets(alpha.size);

    assert(alpha.size > 4); // For the static analyser
    count_buckets(x, alpha.size, buckets);
    TL_ERROR_IF_NEQ_LL(buckets[0], 1LL); // $
    TL_ERROR_IF_NEQ_LL(buckets[1], 4LL); // i
    TL_ERROR_IF_NEQ_LL(buckets[2], 1LL); // m
    TL_ERROR_IF_NEQ_LL(buckets[3], 2LL); // p
    TL_ERROR_IF_NEQ_LL(buckets[4], 4LL); // s

    init_buckets_start(alpha.size, buck_ptr, buckets);
    TL_ERROR_IF_NEQ_LL(buck_ptr[0], 0LL); // $
    TL_ERROR_IF_NEQ_LL(buck_ptr[1], 1LL); // i
    TL_ERROR_IF_NEQ_LL(buck_ptr[2], 5LL); // m
    TL_ERROR_IF_NEQ_LL(buck_ptr[3], 6LL); // p
    TL_ERROR_IF_NEQ_LL(buck_ptr[4], 8LL); // s

    init_buckets_end(alpha.size, buck_ptr, buckets);
    TL_ERROR_IF_NEQ_LL(buck_ptr[0], 1LL);  // $
    TL_ERROR_IF_NEQ_LL(buck_ptr[1], 5LL);  // i
    TL_ERROR_IF_NEQ_LL(buck_ptr[2], 6LL);  // m
    TL_ERROR_IF_NEQ_LL(buck_ptr[3], 8LL);  // p
    TL_ERROR_IF_NEQ_LL(buck_ptr[4], 12LL); // s

    // Cleanup
    free(buckets);
    free(buck_ptr);
    free(x_buf);

    TL_END();
}

TL_TEST(sais_classify_sl_mississippi)
{
    TL_BEGIN();

    // Setup
    cstr_const_sslice u = CSTR_SLICE_STRING0((const char *)"mississippi");
    cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, u);

    cstr_uislice *x_buf = cstr_alloc_uislice(u.len);
    bool ok = cstr_alphabet_map_to_uint(*x_buf, u, &alpha);
    TL_FATAL_IF(!ok);
    cstr_const_uislice x = CSTR_SLICE_CONST_CAST(*x_buf);

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
    free(x_buf);

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

    cstr_sslice *x = cstr_alloc_sslice(n);
    cstr_uislice *u_buf = cstr_alloc_uislice(n);

    assert(x->buf);     // For the static analyser
    assert(u_buf->buf); // For the static analyser

    cstr_bit_vector *is_s = cstr_new_bv(u_buf->len);

    // Tests...
    for (int k = 0; k < 10; k++)
    {
        // len-1 since we don't want to sample the sentinel
        tl_random_string0(*x, letters.buf, (int)letters.len - 1);
        bool ok = cstr_alphabet_map_to_uint(*u_buf, CSTR_SLICE_CONST_CAST(*x), &alpha);
        TL_ERROR_IF(!ok);
        cstr_const_uislice u = CSTR_SLICE_CONST_CAST(*u_buf);

        classify_sl(u, is_s);
        for (long long i = 0; i < is_s->no_bits - 1; i++)
        {
            if (IS_S(i))
            {
                TL_ERROR_IF_GE_SLICE(CSTR_SUFFIX(u, i), CSTR_SUFFIX(u, i + 1));
            }
            else
            {
                TL_ERROR_IF_LE_SLICE(CSTR_SUFFIX(u, i), CSTR_SUFFIX(u, i + 1));
            }
        }
        TL_ERROR_IF_NEQ_INT(IS_S(n - 1), 1);
    }

    free(x);
    free(u_buf);
    free(is_s);

    TL_END();
}

TL_TEST(induce_mississippi)
{
    TL_BEGIN();

    cstr_const_sslice u = CSTR_SLICE_STRING0((const char *)"mississippi");
    cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, u);

    cstr_uislice *x_buf = cstr_alloc_uislice(u.len);
    cstr_uislice *sa = cstr_alloc_uislice(u.len);
    bool ok = cstr_alphabet_map_to_uint(*x_buf, u, &alpha);
    TL_ERROR_IF(!ok);
    cstr_const_uislice x = CSTR_SLICE_CONST_CAST(*x_buf);

    cstr_bit_vector *is_s = cstr_new_bv(x.len);

    // Tests...
    classify_sl(x, is_s);
    undefine_sa_slice(*sa);
    for (long long i = 0; i < sa->len; i++)
    {
        TL_ERROR_IF(is_def(sa->buf[i]));
    }

    long long *buckets = alloc_buckets(alpha.size);
    long long *buck_ptr = alloc_buckets(alpha.size);
    count_buckets(x, alpha.size, buckets);
    init_buckets_end(alpha.size, buck_ptr, buckets);
    bucket_lms(x, *sa, is_s, buck_ptr);

    // -S--S--S---S
    // mississippi$
    //  1  4  7   11
    // $-bucket [0,1)
    // i-bucket [1,5) -- [2,5) for LMS
    assert(sa->len > 11); // For the static analysis
    TL_ERROR_IF_NEQ_UINT(sa->buf[0], 11U);
    TL_ERROR_IF_NEQ_UINT(sa->buf[1], UNDEF);
    TL_ERROR_IF_NEQ_UINT(sa->buf[2], 1U);
    TL_ERROR_IF_NEQ_UINT(sa->buf[3], 4U);
    TL_ERROR_IF_NEQ_UINT(sa->buf[4], 7U);
    TL_ERROR_IF_NEQ_UINT(sa->buf[5], UNDEF);
    TL_ERROR_IF_NEQ_UINT(sa->buf[6], UNDEF);
    TL_ERROR_IF_NEQ_UINT(sa->buf[7], UNDEF);
    TL_ERROR_IF_NEQ_UINT(sa->buf[8], UNDEF);
    TL_ERROR_IF_NEQ_UINT(sa->buf[9], UNDEF);
    TL_ERROR_IF_NEQ_UINT(sa->buf[10], UNDEF);
    TL_ERROR_IF_NEQ_UINT(sa->buf[11], UNDEF);

    // Cleanup
    free(buckets);
    free(buck_ptr);
    free(is_s);
    free(x_buf);
    free(sa);

    TL_END();
}

TL_TEST(buckets_lms_mississippi)
{
    TL_BEGIN();

    cstr_const_sslice u = CSTR_SLICE_STRING0((const char *)"mississippi");
    cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, u);

    cstr_uislice *x_buf = cstr_alloc_uislice(u.len);
    cstr_uislice *sa = cstr_alloc_uislice(u.len);
    bool ok = cstr_alphabet_map_to_uint(*x_buf, u, &alpha);
    TL_ERROR_IF(!ok);

    cstr_const_uislice x = CSTR_SLICE_CONST_CAST(*x_buf);

    cstr_bit_vector *is_s = cstr_new_bv(x.len);

    // Tests...
    classify_sl(x, is_s);
    undefine_sa_slice(*sa);
    for (long long i = 0; i < sa->len; i++)
    {
        TL_ERROR_IF(is_def(sa->buf[i]));
    }

    long long *buckets = alloc_buckets(alpha.size);
    long long *buck_ptr = alloc_buckets(alpha.size);
    count_buckets(x, alpha.size, buckets);

    init_buckets_end(alpha.size, buck_ptr, buckets);
    bucket_lms(x, *sa, is_s, buck_ptr);

    CSTR_SLICE_PRINT(*sa);
    printf("\n");
    // -S--S--S---S
    // mississippi$
    //  1  4  7   11
    // $-bucket [0,1)
    // i-bucket [1,5) -- [2,5) for LMS
    TL_ERROR_IF_NEQ_UINT(sa->buf[0], 11U);
    TL_ERROR_IF_NEQ_UINT(sa->buf[1], UNDEF);
    TL_ERROR_IF_NEQ_UINT(sa->buf[2], 1U);
    TL_ERROR_IF_NEQ_UINT(sa->buf[3], 4U);
    TL_ERROR_IF_NEQ_UINT(sa->buf[4], 7U);
    TL_ERROR_IF_NEQ_UINT(sa->buf[5], UNDEF);
    TL_ERROR_IF_NEQ_UINT(sa->buf[6], UNDEF);
    TL_ERROR_IF_NEQ_UINT(sa->buf[7], UNDEF);
    TL_ERROR_IF_NEQ_UINT(sa->buf[8], UNDEF);
    TL_ERROR_IF_NEQ_UINT(sa->buf[9], UNDEF);
    TL_ERROR_IF_NEQ_UINT(sa->buf[10], UNDEF);
    TL_ERROR_IF_NEQ_UINT(sa->buf[11], UNDEF);

    init_buckets_start(alpha.size, buck_ptr, buckets);
    induce_l(x, *sa, is_s, buck_ptr);
    CSTR_SLICE_PRINT(*sa);
    printf("\n");
    // -S--S--S---S
    // mississippi$
    //  1  4  7   11
    TL_ERROR_IF_NEQ_UINT(sa->buf[0], 11U); // $
    TL_ERROR_IF_NEQ_UINT(sa->buf[1], 10U); // i$
    TL_ERROR_IF_NEQ_UINT(sa->buf[2], 1U);  // ississippi$
    TL_ERROR_IF_NEQ_UINT(sa->buf[3], 4U);  // issippi$
    TL_ERROR_IF_NEQ_UINT(sa->buf[4], 7U);  // ippi$
    TL_ERROR_IF_NEQ_UINT(sa->buf[5], 0U);  // mississippi$
    TL_ERROR_IF_NEQ_UINT(sa->buf[6], 9U);  // pi$
    TL_ERROR_IF_NEQ_UINT(sa->buf[7], 8U);  // ppi$
    TL_ERROR_IF_NEQ_UINT(sa->buf[8], 3);   // sissippi$
    TL_ERROR_IF_NEQ_UINT(sa->buf[9], 6);   // sippi$
    TL_ERROR_IF_NEQ_UINT(sa->buf[10], 2);  // ssissippi$
    TL_ERROR_IF_NEQ_UINT(sa->buf[11], 5);  // ssippi$

    init_buckets_end(alpha.size, buck_ptr, buckets);
    induce_s(x, *sa, is_s, buck_ptr);
    CSTR_SLICE_PRINT(*sa);
    printf("\n");

    // -S--S--S---S
    // mississippi$
    //  1  4  7   11
    // $-bucket [0,1)
    // i-bucket [1,5) -- [2,5) for LMS
    TL_ERROR_IF_NEQ_UINT(sa->buf[0], 11U); // $
    TL_ERROR_IF_NEQ_UINT(sa->buf[1], 10U); // i$
    TL_ERROR_IF_NEQ_UINT(sa->buf[2], 7U);  // ippi$
    TL_ERROR_IF_NEQ_UINT(sa->buf[3], 1U);  // ississippi$
    TL_ERROR_IF_NEQ_UINT(sa->buf[4], 4U);  // issippi$
    TL_ERROR_IF_NEQ_UINT(sa->buf[5], 0U);  // mississippi$
    TL_ERROR_IF_NEQ_UINT(sa->buf[6], 9U);  // pi$
    TL_ERROR_IF_NEQ_UINT(sa->buf[7], 8U);  // ppi$
    TL_ERROR_IF_NEQ_UINT(sa->buf[8], 3);   // sissippi$
    TL_ERROR_IF_NEQ_UINT(sa->buf[9], 6);   // sippi$
    TL_ERROR_IF_NEQ_UINT(sa->buf[10], 2);  // ssissippi$
    TL_ERROR_IF_NEQ_UINT(sa->buf[11], 5);  // ssippi$

    // Cleanup
    free(buckets);
    free(buck_ptr);
    free(is_s);

    free(x_buf);
    free(sa);

    TL_END();
}

#endif
