#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "cstr.h"

// clang-format off
const uint16_t LOW_BIT_MASK = (uint16_t)0xff;
const uint16_t HIGH_BIT_MASK = (uint16_t)~LOW_BIT_MASK;
const uint16_t UNDEFINED = HIGH_BIT_MASK;
const uint16_t DEFINED = LOW_BIT_MASK; // Where we use this one, we just don't want high bits
static inline bool is_undef(uint16_t b) { return b & HIGH_BIT_MASK; } // undef if high bits
static inline bool is_def(uint16_t b)   { return !is_undef(b); }      // otherwise defined
static inline uint8_t byte(uint16_t b)  { return b & LOW_BIT_MASK; }

#define CHECK_VALID(b) if (is_undef(b)) goto error;
// clang-format on

void cstr_init_alphabet(cstr_alphabet *alpha, cstr_const_sslice slice)
{
    // initialise the maps to a non-byte. We can check if a byte is in the
    // map by checking if the higher bits are zero.
    for (int i = 0; i < CSTR_MAX_ALPHABET_SIZE; i++)
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
    for (int i = 0; i < CSTR_MAX_ALPHABET_SIZE; i++)
    {
        if (is_def(alpha->map[i]))
        {
            alpha->map[i] = (uint8_t)alpha->size++;
        }
    }

    // Finally, construct the reverse map
    for (int i = 0; i < CSTR_MAX_ALPHABET_SIZE; i++)
    {
        if (is_def(alpha->map[i]))
        {
            alpha->revmap[alpha->map[i]] = (uint8_t)i;
        }
    }
}

bool cstr_alphabet_map(
    cstr_sslice dst,
    cstr_const_sslice src,
    cstr_alphabet const *alpha)
{
    assert(dst.len == src.len);

    for (int i = 0; i < src.len; i++)
    {
        uint16_t map = alpha->map[src.buf[i]];
        CHECK_VALID(map);
        dst.buf[i] = byte(map);
    }

    return true;

error:
    return false;
}

bool cstr_alphabet_map_to_uint(
    cstr_uislice dst,
    cstr_const_sslice src,
    cstr_alphabet const *alpha)
{
    assert(dst.buf);
    assert(src.buf);
    assert(dst.len == src.len);

    for (int i = 0; i < src.len; i++)
    {
        uint16_t map = alpha->map[(unsigned char)src.buf[i]];
        CHECK_VALID(map);
        dst.buf[i] = byte(map);
    }

    return true;

error:
    return false;
}

bool cstr_alphabet_revmap(
    cstr_sslice dst,
    cstr_const_sslice src,
    cstr_alphabet const *alpha)
{
    assert(src.buf && dst.buf);
    assert(dst.len == src.len);

    for (int i = 0; i < src.len; i++)
    {
        uint16_t map = alpha->revmap[src.buf[i]];
        CHECK_VALID(map);
        dst.buf[i] = byte(map);
    }

    return true;

error:
    return false;
}
