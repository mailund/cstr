#include "misc.h"
#include <stdlib.h>

int * string_to_int(char const *x, int n)
{
    int *ix = malloc(n * sizeof *ix);
    for (int i = 0; i < n; i++) {
        ix[i] = x[i];
    }
    return ix;
}
