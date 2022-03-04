#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cstr.h"

uint8_t* cstr_bwt(long long n, uint8_t const* x, unsigned int sa[n])
{
    uint8_t* b = malloc((size_t)(n + 1) * sizeof *b);
    for (int i = 0; i < n; i++) {
        b[i] = (sa[i] == 0) ? 0 : x[sa[i] - 1];
    }
    return b;
}

struct cstr_bwt_c_table* cstr_compute_bwt_c_table(long long n, uint8_t const* x, int asize)
{
    struct cstr_bwt_c_table* ctab = malloc(offsetof(struct cstr_bwt_c_table, cumsum) + (size_t)asize * sizeof(*ctab->cumsum));
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

void cstr_print_bwt_c_table(struct cstr_bwt_c_table const* ctab)
{
    printf("[ ");
    for (int i = 0; i < ctab->asize; i++) {
        printf("%d ", ctab->cumsum[i]);
    }
    printf("]\n");
}

struct cstr_bwt_o_table {
    int asize;
    long long n;
    long long table[];
};

#define OTAB(otab, a, i) ((otab)->table[(i) * (otab)->asize + (a)])

struct cstr_bwt_o_table*
cstr_compute_bwt_o_table(long long n, uint8_t const* bwt,
                         struct cstr_bwt_c_table const* ctab)
{
    struct cstr_bwt_o_table* otab = malloc(offsetof(struct cstr_bwt_o_table, table) + (size_t)(ctab->asize * n) * sizeof(*otab->table));
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

void cstr_print_bwt_o_table(struct cstr_bwt_o_table const* otab)
{
    for (int a = 0; a < otab->asize; a++) {
        for (int i = 0; i < otab->n; i++) {
            printf("%lld ", OTAB(otab, a, i));
        }
        printf("\n");
    }
}

long long cstr_bwt_o_tab_rank(struct cstr_bwt_o_table const* otab, uint8_t a, long long i)
{
    if (i == 0)
        return 0;
    return OTAB(otab, a, i - 1);
}

// FIXME: p should be a slice!
void cstr_bwt_search(
    long long* left, long long* right,
    uint8_t const* x, uint8_t const* p,
    struct cstr_bwt_c_table const* ctab,
    struct cstr_bwt_o_table const* otab)
{
#define C(a) cstr_bwt_c_tab_rank(ctab, a)
#define O(a, i) cstr_bwt_o_tab_rank(otab, a, i)

    *left = 0, *right = otab->n;
    int m = (int)strlen((const char *)p);
    for (int i = m - 1; i >= 0; i--) {
        uint8_t a = p[i];
        *left = C(a) + O(a, *left);
        *right = C(a) + O(a, *right);
        if (*left >= *right)
            break;
    }

#undef C
#undef O
}
