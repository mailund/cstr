#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "cstr_internal.h"

void cstr_init_alphabet(alpha* alpha, csslice slice)
{
    // initialise the maps to zero
    memset(alpha->map, 0, CSTR_NO_CHARS);
    memset(alpha->revmap, 0, CSTR_NO_CHARS);

    // First, figure out which characters we have in our string
    for (int i = 0; i < slice.len; i++) {
        alpha->map[(unsigned char)slice.buf[i]] = 1;
    }

    // then give those letters a number, starting with 1 to reserve
    // the sentinel
    alpha->size = 1;
    for (int i = 0; i < CSTR_NO_CHARS; i++) {
        if (alpha->map[i]) {
            alpha->map[i] = (unsigned char)alpha->size++;
        }
    }

    // Finally, construct the reverse map
    for (int i = 0; i < CSTR_NO_CHARS; i++) {
        if (alpha->map[i]) {
            alpha->revmap[alpha->map[i]] = (unsigned char)i;
        }
    }
}

bool cstr_alphabet_map(
    sslice dst,
    csslice src,
    alpha const* alpha,
    errcodes* err)
{
    bool ok = false;

    clear_error();
    error_if(dst.len != src.len, CSTR_SIZE_ERROR);

    for (int i = 0; i < src.len; i++) {
        unsigned char map = alpha->map[(unsigned char)src.buf[i]];
        error_if(!map && src.buf[i], CSTR_MAPPING_ERROR);
        dst.buf[i] = (char)map;
    }
    ok = true;

error:
    return ok;
}

char* cstr_alphabet_map_new(
    csslice src,
    alpha const* alpha,
    errcodes* err)
{
    clear_error();

    char* dst = malloc(src.len + 1);
    alloc_error_if(!dst);

    bool ok = cstr_alphabet_map(CSTR_SSLICE(dst, src.len), src, alpha, err);
    reraise_error_if(!ok);

    return dst;

error:
    free(dst);
    return 0;
}

bool cstr_alphabet_map_to_int(
    islice dst,
    csslice src,
    alpha const* alpha,
    errcodes* err)
{
    bool ok = false;

    clear_error();
    // the destination should be one longer than the source
    // because we count the sentinel in the int slice.
    error_if(dst.len != src.len + 1, CSTR_SIZE_ERROR);

    // we iterate over s *including* the sentinel!
    for (int i = 0; i < src.len + 1; i++) {
        unsigned char map = alpha->map[(unsigned char)src.buf[i]];
        error_if(!map && src.buf[i], CSTR_MAPPING_ERROR);
        dst.buf[i] = map;
    }
    ok = true;

error:
    return ok;
}

unsigned int* cstr_alphabet_map_to_int_new(
    csslice src,
    alpha const* alpha,
    errcodes* err)
{
    clear_error();

    unsigned int* arr = malloc((src.len + 1) * sizeof(*arr));
    alloc_error_if(!arr);

    bool ok = cstr_alphabet_map_to_int(CSTR_ISLICE(arr, src.len + 1), src, alpha, err);
    reraise_error_if(!ok);

    return arr;

error:
    free(arr);
    return 0;
}

bool cstr_alphabet_revmap(
    sslice dst,
    csslice src,
    alpha const* alpha,
    errcodes* err)
{
    clear_error();
    error_if(dst.len != src.len, CSTR_SIZE_ERROR);

    for (int i = 0; i < src.len; i++) {
        unsigned char map = alpha->revmap[(int)src.buf[i]];
        error_if(!map && src.buf[i], CSTR_MAPPING_ERROR);
        dst.buf[i] = (char)map;
    }

    return true;

error:
    return false;
}

char* cstr_alphabet_revmap_new(
    csslice src,
    alpha const* alpha,
    errcodes* err)
{
    clear_error();

    char* dst = malloc(src.len + 1);
    alloc_error_if(!dst);

    bool ok = cstr_alphabet_revmap(CSTR_SSLICE(dst, src.len), src, alpha, err);
    reraise_error_if(!ok);

    return dst;

error:
    free(dst);
    return 0;
}
