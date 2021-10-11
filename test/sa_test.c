#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>
#include "testlib.h"

void check_suffix_ordered(char const* x, cstr_islice sa)
{
    for (int i = 1; i < sa.len; i++) {
        printf("%s vs %s\n", x + sa.buf[i - 1], x + sa.buf[i]);
        assert(strcmp(x + sa.buf[i - 1], x + sa.buf[i]) < 0);
    }
}

TL_TEST(test_mississippi)
{
    TL_BEGIN();
    
    cstr_sslice x = CSTR_SSLICE_STRING("mississippi");
    
    cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, x);

    cstr_islice mapped = CSTR_ALLOC_ISLICE(x.len + 1);
    // since alpha was created from x we cannot get mapping errors
    // here
    cstr_alphabet_map_to_int(mapped, x, &alpha);
    assert(mapped.buf); // for static analyser
    
    cstr_islice sa = CSTR_ALLOC_ISLICE(x.len + 1);
    assert(sa.buf); // For the static analyser

    cstr_skew(sa, mapped, &alpha);
        
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
