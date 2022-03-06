#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>
#include "testlib.h"
#include "unittests.h"

static void print_rotation(long long n, uint8_t const *x, unsigned int rot)
{
    for (unsigned int i = rot; i < n; i++)
        putchar(x[i]);
    putchar('$');
    for (unsigned int i = 0; i < rot; i++)
        putchar(x[i]);
    putchar('\n');
}

static TL_TEST(bwt_explore_test)
{
    TL_BEGIN();
    
    cstr_alphabet alpha;
    cstr_const_sslice x = CSTR_SLICE_STRING0((const char *)"mississippi");
    cstr_init_alphabet(&alpha, x);

    cstr_uislice *u = cstr_alloc_uislice(x.len);
    cstr_suffix_array *sa = cstr_alloc_uislice(x.len);
    
    cstr_alphabet_map_to_uint(*u, x, &alpha);
    cstr_skew(*sa, CSTR_SLICE_CONST_CAST(*u), &alpha);

    for (int i = 0; i < sa->len; i++)
    {
        print_rotation(sa->len, x.buf, sa->buf[i]);
    }
    printf("\n");

    cstr_sslice *b = cstr_alloc_sslice(x.len);
    cstr_bwt(*b, x, *sa);
    CSTR_SLICE_PRINT(*b);putchar('\n');
    for (int i = 0; i < sa->len; i++)
    {
        putchar(b->buf[i] ? b->buf[i] : '$');
    }
    putchar('\n');
    
    free(u);
    free(sa);
    free(b);
    
    TL_END();
}

static TL_TEST(bwt_reversal)
{
    TL_BEGIN();
    
    cstr_alphabet alpha;
    cstr_const_sslice x = CSTR_SLICE_STRING0((const char *)"mississippi");
    cstr_init_alphabet(&alpha, x);

    cstr_uislice *u = cstr_alloc_uislice(x.len);
    cstr_suffix_array *sa = cstr_alloc_uislice(x.len);
    
    cstr_alphabet_map_to_uint(*u, x, &alpha);
    cstr_sais(*sa, CSTR_SLICE_CONST_CAST(*u), &alpha);

    cstr_sslice *b = cstr_alloc_sslice(x.len);
    cstr_bwt(*b, x, *sa);

    cstr_sslice *r = cstr_alloc_sslice(x.len);
    cstr_reverse_bwt(*r, CSTR_SLICE_CONST_CAST(*b), *sa);
    
    TL_FATAL_IF_NEQ_SLICE(CSTR_SLICE_CONST_CAST(*r), x);
    
    free(u);
    free(sa);
    free(b);
    free(r);
    TL_END();
}


int main(void)
{
    TL_BEGIN_TEST_SUITE("bwt_test");
    TL_RUN_TEST(bwt_explore_test);
    TL_RUN_TEST(bwt_reversal);
    TL_END_SUITE();


#if 0

    struct cstr_bwt_c_table *ctab = cstr_compute_bwt_c_table(x.len, b, 256);
    struct cstr_bwt_o_table *otab = cstr_compute_bwt_o_table(x.len, b, ctab);
    long long left, right;
    cstr_bwt_search(&left, &right, x.buf, (uint8_t *)"is", ctab, otab);
    printf("[%lld,%lld]\n", left, right);
    for (long long i = left; i < right; i++)
    {
        printf("%s\n", x.buf + sa->buf[i]);
    }

    free(otab);
    free(ctab);
    free(b);
    free(sa);
    free(mapped);
#endif
    
    
    return 0;
}
