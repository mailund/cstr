#include "misc.h"
#include <stdlib.h>

int * string_to_int(char const *x, int xlen, bool include_sentinel)
{
    int *ix = malloc((xlen + include_sentinel) * sizeof *ix);
    for (int i = 0; i < xlen; i++) {
        ix[i] = x[i];
    }
    if (include_sentinel) {
        ix[xlen] = SENTINEL;
    }
    return ix;
}
