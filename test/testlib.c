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
void tl_random_string(cstr_sslice x, const uint8_t *alpha, int alpha_size)
{
    for (int i = 0; i < x.len; i++)
    {
        x.buf[i] = alpha[rand() % alpha_size];
    }
}

void tl_random_string0(cstr_sslice x, const uint8_t *alpha, int alpha_size)
{
    for (int i = 0; i < x.len - 1; i++)
    {
        x.buf[i] = alpha[rand() % alpha_size];
    }
    x.buf[x.len - 1] = '\0'; // cppcheck-suppress[unreadVariable]
}

cstr_sslice tl_random_prefix(cstr_sslice x)
{
    // pick non-empty prefix
    assert(x.len > 0);
    int pre = 1 + rand() % (x.len - 1);
    return CSTR_PREFIX(x, pre);
}

cstr_sslice tl_random_suffix(cstr_sslice x)
{
    // pick non-empty suffix
    assert(x.len > 0);
    int suf = rand() % (x.len - 1);
    return CSTR_SUFFIX(x, suf);
}
