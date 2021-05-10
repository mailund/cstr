#ifndef BWT_H
#define BWT_H

char *bwt(int n, char const *x, int sa[n]);

struct c_table {
    int asize;
    int cumsum[];
};
struct c_table *compute_c_table(int n, char const *x, int asize);
void print_c_table(struct c_table const *ctab);
static inline 
int c_tab_rank(struct c_table const *ctab, char i)
{
    return ctab->cumsum[i];
}

struct o_table;
struct o_table *compute_o_table(int n, char const *bwt, struct c_table const *ctab);
void print_o_table(struct o_table const *otab);
int o_tab_rank(struct o_table const *otab, char a, int i);

struct range { int L, R; };
struct range bwt_search(char const *x,
                        char const *p,
                        struct c_table const *ctab,
                        struct o_table const *otab);

#endif
