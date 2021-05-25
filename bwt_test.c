#include "bwt.h"
#include "skew.h"
#include "misc.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static
void print_rotation(int n, char const *x, int rot)
{
    for (int i = rot; i < n; i++) putchar(x[i]);
    putchar('$');
    for (int i = 0; i < rot; i++) putchar(x[i]);
    putchar('\n');
}

int main(void)
{
    char const *x = "mississippi";
    int xlen = strlen(x);
    int n = xlen + 1; // + 1 for sentinel
    int *sa = skew(x, true);
    for (int i = 0; i < n; i++) {
        print_rotation(xlen, x, sa[i]);
    }
    printf("\n");
    
    char *b = bwt(n, x, sa);
    for (int i = 0; i < n; i++) {
        putchar(b[i] ? b[i] : '$');
    }
    putchar('\n');

    struct c_table *ctab = compute_c_table(n, b, 256);
    struct o_table *otab = compute_o_table(n, b, ctab);
    struct range r = bwt_search(x, "is", ctab, otab);
    printf("[%d,%d]\n", r.L, r.R);
    for (int i = r.L; i < r.R; i++) {
        printf("%s\n", x + sa[i]);
    }

    free(otab);
    free(ctab);
    free(b);
    free(sa);
    
    return 0;
}