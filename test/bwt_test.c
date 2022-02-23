#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>

static void print_rotation(long long n, uint8_t const *x, unsigned int rot)
{
    for (unsigned int i = rot; i < n; i++)
        putchar(x[i]);
    putchar('$');
    for (unsigned int i = 0; i < rot; i++)
        putchar(x[i]);
    putchar('\n');
}

int main(void)
{
    cstr_const_sslice x = CSTR_SLICE_STRING((const char *)"mississippi");

    cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, x);

    cstr_uislice mapped = CSTR_ALLOC_SLICE_BUFFER(mapped, x.len);
    cstr_suffix_array sa = CSTR_ALLOC_SLICE_BUFFER(sa, x.len);
    assert(mapped.buf && sa.buf); // For the static analyser

    cstr_alphabet_map_to_uint(mapped, x, &alpha);

    cstr_skew(sa, CSTR_SLICE_CONST_CAST(mapped), &alpha);

    for (int i = 0; i < sa.len; i++)
    {
        print_rotation(sa.len, x.buf, sa.buf[i]);
    }
    printf("\n");

    uint8_t *b = cstr_bwt(x.len, x.buf, sa.buf);
    for (int i = 0; i < sa.len; i++)
    {
        putchar(b[i] ? b[i] : '$');
    }
    putchar('\n');

    struct cstr_bwt_c_table *ctab = cstr_compute_bwt_c_table(x.len, b, 256);
    struct cstr_bwt_o_table *otab = cstr_compute_bwt_o_table(x.len, b, ctab);
    long long left, right;
    cstr_bwt_search(&left, &right, x.buf, (uint8_t *)"is", ctab, otab);
    printf("[%lld,%lld]\n", left, right);
    for (long long i = left; i < right; i++)
    {
        printf("%s\n", x.buf + sa.buf[i]);
    }

    free(otab);
    free(ctab);
    free(b);
    CSTR_FREE_SLICE_BUFFER(sa);
    CSTR_FREE_SLICE_BUFFER(mapped);

    return 0;
}
