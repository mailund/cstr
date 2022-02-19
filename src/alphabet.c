#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "cstr.h"

void cstr_init_alphabet(cstr_alphabet *alpha, cstr_sslice slice)
{
    // initialise the maps to zero
    memset(alpha->map, 0, CSTR_NO_CHARS);
    memset(alpha->revmap, 0, CSTR_NO_CHARS);

    // First, figure out which characters we have in our string
    for (int i = 0; i < slice.len; i++)
    {
        alpha->map[(unsigned char)slice.buf[i]] = 1;
    }

    // Assign consequtive numbers to the letters.
    alpha->size = 0;
    for (int i = 0; i < CSTR_NO_CHARS; i++)
    {
        if (alpha->map[i])
        {
            alpha->map[i] = (unsigned char)alpha->size++;
        }
    }

    // Finally, construct the reverse map
    for (int i = 0; i < CSTR_NO_CHARS; i++)
    {
        if (alpha->map[i])
        {
            alpha->revmap[alpha->map[i]] = (unsigned char)i;
        }
    }
}

bool cstr_alphabet_map(
    cstr_sslice dst,
    cstr_sslice src,
    cstr_alphabet const *alpha)
{
    assert(dst.len == src.len);

    for (int i = 0; i < src.len; i++)
    {
        unsigned char map = alpha->map[(unsigned char)src.buf[i]];
        if (!map && src.buf[i])
            goto error;
        dst.buf[i] = (char)map;
    }

    return true;

error:
    return false;
}

bool cstr_alphabet_map_to_uint(
    cstr_uislice dst,
    cstr_sslice src,
    cstr_alphabet const *alpha)
{
    assert(dst.buf);
    assert(src.buf);
    assert(dst.len == src.len);

    for (int i = 0; i < src.len; i++)
    {
        unsigned char map = alpha->map[(unsigned char)src.buf[i]];
        if (!map && src.buf[i])
            goto error;
        dst.buf[i] = map;
    }

    return true;

error:
    return false;
}

bool cstr_alphabet_revmap(
    cstr_sslice dst,
    cstr_sslice src,
    cstr_alphabet const *alpha)
{
    assert(src.buf && dst.buf);
    assert(dst.len == src.len);

    for (int i = 0; i < src.len; i++)
    {
        unsigned char map = alpha->revmap[(int)src.buf[i]];
        if (!map && src.buf[i])
            goto error;
        dst.buf[i] = (char)map;
    }

    return true;

error:
    return false;
}
