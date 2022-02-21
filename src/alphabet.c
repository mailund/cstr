#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "cstr.h"

#define UNDEFINED (1 << 8)
#define DEFINED 42                  // anything without higher bits would work here
#define IS_UNDEFINED(b) (b & ~0xff) // are there higher bits set?
#define IS_DEFINED(b) !IS_UNDEFINED(b)
#define GET_BYTE(x) ((x)&0xff)

void cstr_init_alphabet(cstr_alphabet *alpha, cstr_sslice slice)
{
    // initialise the maps to a non-byte. We can check if a byte is in the
    // map by checking if the higher bits are zero.
    for (int i = 0; i < CSTR_NO_CHARS; i++)
    {
        alpha->map[i] = UNDEFINED;
        alpha->revmap[i] = UNDEFINED;
    }

    // First, figure out which characters we have in our string
    for (int i = 0; i < slice.len; i++)
    {
        alpha->map[slice.buf[i]] = DEFINED;
    }

    // Assign consequtive numbers to the letters.
    alpha->size = 0;
    for (int i = 0; i < CSTR_NO_CHARS; i++)
    {
        if (IS_DEFINED(alpha->map[i]))
        {
            alpha->map[i] = (uint8_t)alpha->size++;
        }
    }

    // Finally, construct the reverse map
    for (int i = 0; i < CSTR_NO_CHARS; i++)
    {
        if (IS_DEFINED(alpha->map[i]))
        {
            alpha->revmap[alpha->map[i]] = (uint8_t)i;
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
        uint16_t map = alpha->map[src.buf[i]];
        if (IS_UNDEFINED(map))
            goto error;
        dst.buf[i] = GET_BYTE(map);
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
        uint16_t map = alpha->map[(unsigned char)src.buf[i]];
        if (IS_UNDEFINED(map))
            goto error;
        dst.buf[i] = GET_BYTE(map);
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
        uint16_t map = alpha->revmap[src.buf[i]];
        if (IS_UNDEFINED(map))
            goto error;
        dst.buf[i] = GET_BYTE(map);
    }

    return true;

error:
    return false;
}
