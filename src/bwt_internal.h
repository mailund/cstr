#ifndef BWT_INTERNAL_H
#define BWT_INTERNAL_H

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

struct c_table *cstr_build_c_table(cstr_const_sslice x, long long sigma);
struct o_table *cstr_build_o_table(cstr_const_sslice bwt, struct c_table const *ctab);

#endif // BWT_INTERNAL_H
