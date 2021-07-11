#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cstr.h"

char *cstr_bwt(int n, char const *x, unsigned int sa[n]) {
    char *b = malloc((size_t)(n + 1) * sizeof *b);
    for (int i = 0; i < n; i++) {
        b[i] = (sa[i] == 0) ? 0 : x[sa[i] - 1];
    }
    return b;
}

struct cstr_bwt_c_table *cstr_compute_bwt_c_table(int n, char const *x,
                                                  int asize) {
    struct cstr_bwt_c_table *ctab =
        malloc(offsetof(struct cstr_bwt_c_table, cumsum) +
               (size_t)asize * sizeof(*ctab->cumsum));
    ctab->asize = asize;
    for (int i = 0; i < asize; i++)
        ctab->cumsum[i] = 0;
    for (int i = 0; i < n; i++)
        ctab->cumsum[(int)x[i]]++;
    for (int i = 0, acc = 0; i < asize; i++) {
        int k = ctab->cumsum[i];
        ctab->cumsum[i] = acc;
        acc += k;
    }
    return ctab;
}

void cstr_print_bwt_c_table(struct cstr_bwt_c_table const *ctab) {
    printf("[ ");
    for (int i = 0; i < ctab->asize; i++) {
        printf("%d ", ctab->cumsum[i]);
    }
    printf("]\n");
}

struct cstr_bwt_o_table {
    int asize;
    int n;
    int table[];
};

#define OTAB(otab, a, i) ((otab)->table[(i)*(otab)->asize + (a)])

struct cstr_bwt_o_table *
cstr_compute_bwt_o_table(int n, char const *bwt,
                         struct cstr_bwt_c_table const *ctab) {
    struct cstr_bwt_o_table *otab =
        malloc(offsetof(struct cstr_bwt_o_table, table) +
               (size_t)(ctab->asize * n) * sizeof(*otab->table));
    otab->asize = ctab->asize;
    otab->n = n;
    for (int a = 0; a < ctab->asize; a++) {
        OTAB(otab, a, 0) = (a == bwt[0]);
        for (int i = 1; i < n; i++) {
            OTAB(otab, a, i) = OTAB(otab, a, i - 1) + (a == bwt[i]);
        }
    }
    return otab;
}

void cstr_print_bwt_o_table(struct cstr_bwt_o_table const *otab) {
    for (int a = 0; a < otab->asize; a++) {
        for (int i = 0; i < otab->n; i++) {
            printf("%d ", OTAB(otab, a, i));
        }
        printf("\n");
    }
}

int cstr_bwt_o_tab_rank(struct cstr_bwt_o_table const *otab, char a, int i) {
    if (i == 0)
        return 0;
    return OTAB(otab, a, i - 1);
}

struct range cstr_bwt_search(char const *x, char const *p,
                             struct cstr_bwt_c_table const *ctab,
                             struct cstr_bwt_o_table const *otab) {
#define C(a) cstr_bwt_c_tab_rank(ctab, a)
#define O(a, i) cstr_bwt_o_tab_rank(otab, a, i)

    int L = 0, R = otab->n;
    int m = (int)strlen(p);
    for (int i = m - 1; i >= 0; i--) {
        char a = p[i];
        L = C(a) + O(a, L);
        R = C(a) + O(a, R);
        if (L >= R)
            break;
    }
    return (struct range){.L = L, .R = R};

#undef C
#undef O
}
