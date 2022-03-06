#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cstr.h"

struct c_table
{
    long long sigma;
    unsigned int cumsum[];
};
#define C(A) (ctab->cumsum[A])

struct o_table
{
    long long sigma;
    long long n;
    long long table[];
};
// Direct look-up, not compensating for row zero
#define O_RAW(A, I) (otab->table[(I) * (otab)->sigma + (A)])
// Correcting for implict zero row
#define O(A, I) (((I) == 0) ? 0 : O_RAW((A), (I)-1))


/*
 static void cstr_print_bwt_c_table(struct c_table const *ctab)
 {
     printf("[ ");
     for (long long i = 0; i < ctab->sigma; i++)
     {
         printf("%u ", ctab->cumsum[i]);
     }
     printf("]\n");
 }

 static void cstr_print_bwt_o_table(struct o_table const *otab)
 {
     for (int a = 0; a < otab->sigma; a++)
     {
         for (int i = 0; i < otab->n; i++)
         {
             printf("%lld ", O_RAW(a, i));
         }
         printf("\n");
     }
 }
*/

void cstr_bwt(cstr_sslice bwt, cstr_const_sslice x, cstr_suffix_array sa)
{
    assert(bwt.len == x.len && x.len == sa.len);
    for (long long i = 0; i < x.len; i++)
    {
        // Previous index, with wrap at index zero...
        long long j = sa.buf[i] + (sa.buf[i] == 0) * x.len;
        bwt.buf[i] = x.buf[j - 1];
    }
}

static struct c_table *build_c_table(cstr_const_sslice x, long long sigma)
{
    struct c_table *ctab = CSTR_MALLOC_FLEX_ARRAY(ctab, cumsum, (size_t)sigma);
    ctab->sigma = sigma;
    for (long long i = 0; i < sigma; i++)
        ctab->cumsum[i] = 0;
    for (long long i = 0; i < x.len; i++)
        ctab->cumsum[x.buf[i]]++;
    for (long long i = 0, acc = 0; i < sigma; i++)
    {
        unsigned int k = ctab->cumsum[i];
        ctab->cumsum[i] = (unsigned int)acc;
        acc += k;
    }
    return ctab;
}

static struct o_table *build_o_table(cstr_const_sslice bwt, struct c_table const *ctab)
{
    struct o_table *otab = CSTR_MALLOC_FLEX_ARRAY(otab, table, (size_t)ctab->sigma * (size_t)bwt.len);
    otab->sigma = ctab->sigma;
    otab->n = bwt.len;
    for (long long a = 0; a < ctab->sigma; a++)
    {
        O_RAW(a, 0) = (a == bwt.buf[0]);
        for (int i = 1; i < bwt.len; i++)
        {
            O_RAW(a, i) = O_RAW(a, i - 1) + (a == bwt.buf[i]);
        }
    }
    return otab;
}

void cstr_reverse_bwt(cstr_sslice rev, cstr_const_sslice bwt, cstr_suffix_array sa)
{
    assert(rev.len == bwt.len && bwt.len == sa.len);

    // Use the alphabet code to get the size of the alphabet, sigma.
    cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, bwt);
    long long sigma = alpha.size;
    
    // We don't assume that the bwt string is mapped down to the alphabet here (although
    // maybe we should to save some time), so we also need to do that...
    cstr_sslice *mapped_buf = cstr_alloc_sslice(bwt.len);
    cstr_alphabet_map(*mapped_buf, bwt, &alpha);
    cstr_const_sslice mapped = CSTR_SLICE_CONST_CAST(*mapped_buf);

    struct c_table *ctab = build_c_table(mapped, sigma);
    struct o_table *otab = build_o_table(mapped, ctab);

    rev.buf[rev.len - 1] = 0; // Sentinel at the end of rev.
    // The sentinel is also at the beginning of row 0 in the BWT
    // matrix, so i starts at zero.
    for (long long i = 0, j = rev.len - 2; j >= 0; j--)
    {
        uint8_t a = mapped.buf[i];
        rev.buf[j] = a;
        i = C(a) + O(a, i);
    }
    
    // If the input wasn't mapped down to the alphabet, then rev isn't in the
    // right alphabet either, and so we need to map it back.
    cstr_alphabet_revmap(rev, CSTR_SLICE_CONST_CAST(rev), &alpha);

    free(mapped_buf);
    free(ctab);
    free(otab);
}

/*


// FIXME: p should be a slice!
void cstr_bwt_search(
    long long *left, long long *right,
    uint8_t const *x, uint8_t const *p,
    struct cstr_bwt_c_table const *ctab,
    struct cstr_bwt_o_table const *otab)
{
    *left = 0, *right = otab->n;
    int m = (int)strlen((const char *)p);
    for (int i = m - 1; i >= 0; i--)
    {
        uint8_t a = p[i];
        *left = C(a) + O(a, *left);
        *right = C(a) + O(a, *right);
        if (*left >= *right)
            break;
    }
}

*/
