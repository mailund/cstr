#include <stdlib.h>

#include "cstr.h"

void cstr_refcount_object_init(struct cstr_refcount_object *obj,
                               struct cstr_refcount_type *type) {
    obj->type = type;
    obj->refcount = 1;
}

void *cstr_refcount_incref(void *obj) {
    if (obj) {
        ((struct cstr_refcount_object *)obj)->refcount++;
    }
    return obj;
}

void *cstr_refcount_decref(void *obj) {
    if (!obj) {
        return obj;
    }
    struct cstr_refcount_object *rcobj = obj;
    if (--(rcobj->refcount) == 0) {
        rcobj->type->cleanup(rcobj);
        obj = 0;
    }
    return obj;
}
