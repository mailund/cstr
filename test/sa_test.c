#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>
#include <cstr_internal.h>
#include "testlib.h"

void check_suffix_ordered(char const* x, struct cstr_islice sa)
{
    for (int i = 1; i < sa.len; i++) {
        printf("%s vs %s\n", x + sa.buf[i - 1], x + sa.buf[i]);
        assert(strcmp(x + sa.buf[i - 1], x + sa.buf[i]) < 0);
    }
}

TL_TEST(test_mississippi)
{
    TL_BEGIN();
    
    struct cstr_sslice x = CSTR_SSLICE_STRING("mississippi");
    enum cstr_errcodes err;

    struct cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, x);

    struct cstr_islice sa = CSTR_ALLOC_ISLICE(x.len + 1);
    assert(sa.buf); // For the static analyser

    bool ok = cstr_skew(sa, x, &alpha, &err);
    TL_FATAL_IF(!ok);
    TL_FATAL_IF(err != CSTR_NO_ERROR);
    
    
    for (int i = 0; i < x.len + 1; i++) {
        printf("sa[%d] == %d %s\n", i, sa.buf[i], x.buf + sa.buf[i]);
    }

    check_suffix_ordered(x.buf, sa);

    CSTR_FREE_SLICE_BUFFER(sa);

    TL_END();
}

int main(void)
{
    TL_BEGIN_TEST_SUITE("sa_test");
    TL_RUN_TEST(test_mississippi);
    TL_END_SUITE();
}
