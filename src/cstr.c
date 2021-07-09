#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// Here we make sure that we emit the inline
// functions in the header.
#define INLINE extern inline
#include "cstr.h"

// -- cstr type --------------
static struct cstr_refcount_type cstr_type = {.cleanup = free};

struct cstr *cstr_alloc_cstr(size_t len) {
    struct cstr *cstr = malloc(offsetof(struct cstr, buf) + len + 1);
    if (!cstr) {
        perror("couldn't allocate cstr");
        exit(1);
    }

    cstr_refcount_object_init(&cstr->rc, &cstr_type);
    cstr->len = len;
    cstr->buf[len] = '\0'; // always zero-terminate

    return cstr;
}

struct cstr *cstr_cstr_from_string(char *x) {
    struct cstr *cstr = cstr_alloc_cstr(strlen(x));
    strcpy(cstr->buf, x);
    return cstr;
}

// -- int array type --------------
static struct cstr_refcount_type int_array_type = {.cleanup = free};

struct cstr_int_array *cstr_alloc_int_array(size_t len) {
    struct cstr_int_array *arr =
        malloc(offsetof(struct cstr_int_array, buf) + len * sizeof(*arr));
    if (!arr) {
        perror("couldn't allocate int array");
        exit(1);
    }

    cstr_refcount_object_init(&arr->rc, &int_array_type);
    arr->len = len;

    return arr;
}
