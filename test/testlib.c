#include "testlib.h"

#include <stdlib.h>
#include <string.h>

int tl_test_array(void *restrict expected, void *restrict actual, size_t arrlen,
                  size_t objsize)
{
    char *restrict x = expected, *restrict a = actual;
    for (int i = 0; i < arrlen; i++)
    {
        if (memcmp(x, a, objsize) != 0)
        {
            return i;
        }
        x += objsize;
        a += objsize;
    }
    return -1;
}

// test strings
void tl_random_string(cstr_sslice x, const char *alpha, int alpha_size)
{
    for (int i = 0; i < x.len; i++)
    {
        x.buf[i] = alpha[rand() % alpha_size];
    }
}

cstr_sslice tl_random_prefix(cstr_sslice x)
{
    int k = rand() % (x.len - 1); // pick non-empty prefix
    return CSTR_SSLICE(x.buf, x.len - k);
}
