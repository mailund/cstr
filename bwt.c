#include "bwt.h"
#include "misc.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>


char *bwt(int n, char const *x, int sa[n])
{
    char *b = malloc((n + 1) * sizeof *b);
    for (int i = 0; i < n; i++) {
        b[i] = (sa[i] == 0) ? SENTINEL : x[sa[i] - 1];
    }
    return b;
}

struct c_table *compute_c_table(int n, char const *x, int asize)
{
    struct c_table *ctab = malloc(offsetof(struct c_table, cumsum) + asize * sizeof *ctab->cumsum);
    ctab->asize = asize;
    for (int i = 0; i < asize; i++) ctab->cumsum[i] = 0;
    for (int i = 0; i < n; i++)     ctab->cumsum[x[i]]++;
    for (int i = 0, acc = 0; i < asize; i++) {
        int k = ctab->cumsum[i];
        ctab->cumsum[i] = acc;
        acc += k;
    }
    return ctab;
}

void print_c_table(struct c_table const *ctab)
{
    printf("[ ");
    for (int i = 0; i < ctab->asize; i++) {
        printf("%d ", ctab->cumsum[i]);
    }
    printf("]\n");
}

struct o_table {
    int asize;
    int n;
    int table[];
};

static inline
int *otab_idx(struct o_table *otab, int a, int i)
{
    return &otab->table[i * otab->asize + a];
}

struct o_table *compute_o_table(int n, char const *bwt, struct c_table const *ctab)
{
    struct o_table *otab = malloc(offsetof(struct o_table, table) + ctab->asize * n * sizeof *otab->table);
    otab->asize = ctab->asize;
    otab->n = n;
    for (int a = 0; a < ctab->asize; a++) {
        *otab_idx(otab, a, 0) = (a == bwt[0]);
        for (int i = 1; i < n; i++) {
            *otab_idx(otab, a, i) = *otab_idx(otab, a, i - 1) + (a == bwt[i]);
        }
    }
    return otab;
}

void print_o_table(struct o_table const *otab)
{
    for (int a = 0; a < otab->asize; a++) {
        for (int i = 0; i < otab->n; i++) {
            printf("%d ", *otab_idx((struct o_table *)otab, a, i));
        }
        printf("\n");
    }
}

int o_tab_rank(struct o_table const *otab, char a, int i)
{
    if (i == 0) return 0;
    return *otab_idx((struct o_table *)otab, a, i - 1);
}

struct range bwt_search(char const *x,
                        char const *p,
                        struct c_table const *ctab,
                        struct o_table const *otab)
{
#define C(a)   c_tab_rank(ctab, a)
#define O(a,i) o_tab_rank(otab, a, i)

    int L = 0, R = otab->n;
    int m = strlen(p);
    for (int i = m - 1; i >= 0; i--) {
        char a = p[i];
        L = C(a) + O(a, L);
        R = C(a) + O(a, R);
        if (L >= R) break;
    }
    return (struct range){ .L = L, .R = R };

#undef C
#undef O
}