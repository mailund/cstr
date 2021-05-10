#include "skew.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    char const *x = "mississippi";
    int n = strlen(x);
    int *sa = skew(x);
    for (int i = 0; i < n; i++) {
        printf("%2d: %s\n", sa[i], x + sa[i]);
    }
    free(sa);
    return 0;
}
