#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>

void check_suffix_ordered(char const* x, unsigned int n, unsigned int sa[n])
{
    for (int i = 1; i < n; i++) {
        printf("%s vs %s\n", x + sa[i - 1], x + sa[i]);
        assert(strcmp(x + sa[i - 1], x + sa[i]) < 0);
    }
}

void test_mississippi()
{
    struct cstr_const_sslice x = CSTR_CSSLICE_STRING("mississippi");
    enum cstr_errcodes err;

    struct cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, x);
    unsigned int* sa = cstr_skew_new(x, &alpha, &err);
    assert(sa != NULL);
    assert(CSTR_NO_ERROR == err);
    
    for (int i = 0; i < x.len + 1; i++) {
        printf("sa[%d] == %d %s\n", i, sa[i], x.buf + sa[i]);
    }

    check_suffix_ordered(x.buf, x.len + 1, sa);
    free(sa);
}

int main(void)
{
    test_mississippi();
    return 0;
}
