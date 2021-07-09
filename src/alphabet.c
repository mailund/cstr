#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "cstr.h"

static struct cstr_refcount_type alphabet_type = {.cleanup = free};

static struct cstr_alphabet *alloc_alphabet(void) {
    struct cstr_alphabet *alpha = malloc(sizeof *alpha);
    if (!alpha) {
        perror("couldn't allocate alphabet");
        exit(1);
    }

    cstr_refcount_object_init(&alpha->rc, &alphabet_type);

    memset(alpha->map, 0, 256);
    memset(alpha->revmap, 0, 256);

    return alpha;
}

struct cstr_alphabet *cstr_alphabet_from_slice(struct cstr_str_slice slice) {
    struct cstr_alphabet *alpha = alloc_alphabet();

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
}

struct cstr *cstr_alphabet_map(struct cstr_alphabet const *alpha,
                               struct cstr_str_slice s) {
    struct cstr *cstr = cstr_alloc_cstr(s.len);
    for (int i = 0; i < s.len; i++) {
        char map = alpha->map[s.buf[i]];
        if (!map && s.buf[i]) {
            // only the sentinel can map to zero
            goto error;
        }
        cstr->buf[i] = map;
    }

    return cstr;

error:
    cstr_refcount_decref(cstr);
    return 0;
}

struct cstr_int_array *
cstr_alphabet_map_to_int(struct cstr_alphabet const *alpha,
                         struct cstr_str_slice s) {
    struct cstr_int_array *arr =
        cstr_alloc_int_array(s.len + 1); // + 1 for sentinel.
    for (int i = 0; i < s.len + 1; i++) {
        char map = alpha->map[s.buf[i]];
        if (!map && s.buf[i]) { // only the sentinel can map to zero
            goto error;
        }
        arr->buf[i] = map;
    }

    return arr;

error:
    cstr_refcount_decref(arr);
    return 0;
}

struct cstr *cstr_alphabet_revmap(struct cstr_alphabet const *alpha,
                                  struct cstr_str_slice s) {
    struct cstr *cstr = cstr_alloc_cstr(s.len);
    for (int i = 0; i < s.len; i++) {
        char map = alpha->revmap[s.buf[i]];
        if (!map && s.buf[i]) {
            // only the sentinel can map to zero
            goto error;
        }
        cstr->buf[i] = map;
    }

    return cstr;

error:
    cstr_refcount_decref(cstr);
    return 0;
}
