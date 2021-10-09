#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "cstr_internal.h"

void cstr_init_alphabet(alpha *alpha, sslice slice)
{
    // initialise the maps to zero
    memset(alpha->map, 0, CSTR_NO_CHARS);
    memset(alpha->revmap, 0, CSTR_NO_CHARS);

    // First, figure out which characters we have in our string
    for (int i = 0; i < slice.len; i++)
    {
        alpha->map[(unsigned char)slice.buf[i]] = 1;
    }

    // then give those letters a number, starting with 1 to reserve
    // the sentinel
    alpha->size = 1;
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
    sslice dst,
    sslice src,
    alpha const *alpha,
    errcodes *err)
{
    bool ok = false;

    clear_error();
    size_error_if(dst.len != src.len, done);

    for (int i = 0; i < src.len; i++)
    {
        unsigned char map = alpha->map[(unsigned char)src.buf[i]];
        mapping_error_if(!map && src.buf[i], done);
        dst.buf[i] = (char)map;
    }
    ok = true;

done:
    return ok;
}


bool cstr_alphabet_map_to_int(
    islice dst,
    sslice src,
    alpha const *alpha,
    errcodes *err)
{
    assert(dst.buf);
    assert(src.buf);
    
    bool ok = false;

    clear_error();
    // the destination should be one longer than the source
    // because we count the sentinel in the int slice.
    size_error_if(dst.len != src.len + 1, done);

    for (int i = 0; i < src.len; i++)
    {
        unsigned char map = alpha->map[(unsigned char)src.buf[i]];
        mapping_error_if(!map && src.buf[i], done);
        dst.buf[i] = map;
    }
    // remember sentinel
    dst.buf[src.len] = 0;
    
    ok = true;

done:
    return ok;
}

bool cstr_alphabet_revmap(
    sslice dst,
    sslice src,
    alpha const *alpha,
    errcodes *err)
{
    assert(src.buf && dst.buf);
    
    clear_error();
    size_error_if(dst.len != src.len, error);

    for (int i = 0; i < src.len; i++)
    {
        unsigned char map = alpha->revmap[(int)src.buf[i]];
        mapping_error_if(!map && src.buf[i], error);
        dst.buf[i] = (char)map;
    }

    return true;

error:
    return false;
}

