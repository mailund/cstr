#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>

void check_suffix_ordered(char const* x, struct cstr_islice sa)
{
    for (int i = 1; i < sa.len; i++) {
        printf("%s vs %s\n", x + sa.buf[i - 1], x + sa.buf[i]);
        assert(strcmp(x + sa.buf[i - 1], x + sa.buf[i]) < 0);
    }
}

void test_mississippi()
{
    struct cstr_const_sslice x = CSTR_CSSLICE_STRING("mississippi");
    enum cstr_errcodes err;

    struct cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, x);

    struct cstr_islice sa = CSTR_NIL_SLICE;
    bool ok = cstr_alloc_islice_buffer(&sa, x.len + 1);
    assert(ok);

    ok = cstr_skew(sa, x, &alpha, &err);
    assert(ok);
    assert(err == CSTR_NO_ERROR);

    for (int i = 0; i < x.len + 1; i++) {
        printf("sa[%d] == %d %s\n", i, sa.buf[i], x.buf + sa.buf[i]);
    }

    check_suffix_ordered(x.buf, sa);

    cstr_free_islice_buffer(&sa);
}

int main(void)
{
    test_mississippi();
    return 0;
}
