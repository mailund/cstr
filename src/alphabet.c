#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "cstr_internal.h"

static alpha *alloc_alphabet(void) {
    alpha *alpha = malloc(sizeof *alpha);
    if (alpha) {
        memset(alpha->map, 0, 256);
        memset(alpha->revmap, 0, 256);
    }
    return alpha;
}

alpha *cstr_alphabet_from_slice(sslice slice, errcodes *err) {
    clear_error();

    alpha *alpha = alloc_alphabet();
    alloc_error_if(!alpha);

    // First, figure out which characters we have in our string
    for (int i = 0; i < slice.len; i++) {
        alpha->map[slice.buf[i]] = 1;
    }

    // then give those letters a number, starting with 1 to reserve
    // the sentinel
    alpha->size = 1;
    for (int i = 0; i < 256; i++) {
        if (alpha->map[i]) {
            alpha->map[i] = alpha->size++;
        }
    }

    // Finally, construct the reverse map
    for (int i = 0; i < 256; i++) {
        if (alpha->map[i]) {
            alpha->revmap[alpha->map[i]] = i;
        }
    }

    return alpha;

error:
    return 0;
}

char *cstr_alphabet_map(alpha const *alpha, sslice s, errcodes *err) {
    clear_error();

    char *x = malloc(s.len + 1);
    alloc_error_if(!x);

    for (int i = 0; i < s.len; i++) {
        char map = alpha->map[s.buf[i]];
        error_if(!map && s.buf[i],
                 CSTR_MAPPING_ERROR); // only the sentinel can map to zero
        x[i] = map;
    }

    return x;

error:
    free(x);
    return 0;
}

int *cstr_alphabet_map_to_int(alpha const *alpha, sslice s, errcodes *err) {
    clear_error();

    int *arr = malloc((s.len + 1) * sizeof(*arr));
    alloc_error_if(!arr);

    // we iterate over s *including* the sentinel!
    for (int i = 0; i < s.len + 1; i++) {
        char map = alpha->map[s.buf[i]];
        error_if(!map && s.buf[i],
                 CSTR_MAPPING_ERROR); // only the sentinel can map to zero
        arr[i] = map;
    }

    return arr;

error:
    free(arr);
    return 0;
}

char *cstr_alphabet_revmap(alpha const *alpha, sslice s, errcodes *err) {
    clear_error();

    char *x = malloc(s.len + 1);
    alloc_error_if(!x);

    for (int i = 0; i < s.len; i++) {
        char map = alpha->revmap[s.buf[i]];
        error_if(!map && s.buf[i],
                 CSTR_MAPPING_ERROR); // only the sentinel can map to zero
        x[i] = map;
    }

    return x;

error:
    free(x);
    return 0;
}
