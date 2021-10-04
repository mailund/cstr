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
void tl_random_string(const char *alpha, int alpha_size, char *buf,
                      int buf_len)
{
    for (int i = 0; i < buf_len - 1; i++)
    {
        *buf++ = alpha[rand() % alpha_size];
    }
    *buf = '\0';
}
