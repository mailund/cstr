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
    char const* x = "mississippi";
    int xlen = strlen(x);
    int n = xlen + 1; // + 1 for sentinel
    
    // FIXME: clean this up!
    struct cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, CSTR_CSSLICE_STRING(x));
    unsigned int* sa = cstr_skew_new(CSTR_CSSLICE_STRING(x), &alpha, 0);
    
    for (int i = 0; i < n; i++) {
        print_rotation(xlen, x, sa[i]);
    }
    printf("\n");

    char* b = cstr_bwt(n, x, sa);
    for (int i = 0; i < n; i++) {
        putchar(b[i] ? b[i] : '$');
    }
    putchar('\n');

    struct cstr_bwt_c_table* ctab = cstr_compute_bwt_c_table(n, b, 256);
    struct cstr_bwt_o_table* otab = cstr_compute_bwt_o_table(n, b, ctab);
    int left, right;
    cstr_bwt_search(&left, &right, x, "is", ctab, otab);
    printf("[%d,%d]\n", left, right);
    for (int i = left; i < right; i++) {
        printf("%s\n", x + sa[i]);
    }

    free(otab);
    free(ctab);
    free(b);
    free(sa);

    return 0;
}
