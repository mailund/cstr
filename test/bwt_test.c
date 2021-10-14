#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>

static void print_rotation(int n, char const* x, int rot)
{
    for (int i = rot; i < n; i++)
        putchar(x[i]);
    putchar('$');
    for (int i = 0; i < rot; i++)
        putchar(x[i]);
    putchar('\n');
}

int main(void)
{
    cstr_sslice x = CSTR_SLICE_STRING("mississippi");

    cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, x);

    cstr_uislice mapped = CSTR_ALLOC_SLICE_BUFFER(mapped, x.len + 1);
    cstr_suffix_array sa = CSTR_ALLOC_SLICE_BUFFER(sa, x.len + 1);
    assert(mapped.buf && sa.buf); // For the static analyser

    cstr_alphabet_map_to_uint(mapped, x, &alpha);
    
    cstr_skew(sa, mapped, &alpha);
    
    for (int i = 0; i < sa.len; i++) {
        print_rotation(sa.len, x.buf, sa.buf[i]);
    }
    printf("\n");

    char* b = cstr_bwt(x.len, x.buf, sa.buf);
    for (int i = 0; i < sa.len; i++) {
        putchar(b[i] ? b[i] : '$');
    }
    putchar('\n');

    struct cstr_bwt_c_table* ctab = cstr_compute_bwt_c_table(x.len, b, 256);
    struct cstr_bwt_o_table* otab = cstr_compute_bwt_o_table(x.len, b, ctab);
    int left, right;
    cstr_bwt_search(&left, &right, x.buf, "is", ctab, otab);
    printf("[%d,%d]\n", left, right);
    for (int i = left; i < right; i++) {
        printf("%s\n", x.buf + sa.buf[i]);
    }

    free(otab);
    free(ctab);
    free(b);
    CSTR_FREE_SLICE_BUFFER(sa);
    CSTR_FREE_SLICE_BUFFER(mapped);

    return 0;
}
