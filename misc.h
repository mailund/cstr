#ifndef MISC_H
#define MISC_H

#include <stdbool.h>

#define SENTINEL 0

int * string_to_int(char const *x, int xlen, bool include_sentinel);

#endif
