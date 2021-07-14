#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>

static void print_rotation(int n, char const* x, int rot)
{
    for (int i = rot; i < n; i++)
        putchar(x[i]);
    putchar('$');
    for (int i = 0; i < rot; i++)
        putchar(x[i]);
    putchar('\n');
}

int main(void)
{
    struct cstr_const_sslice x = CSTR_CSSLICE_STRING("mississippi");

    struct cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, x);

    struct cstr_islice sa = CSTR_NIL_SLICE;
    bool ok = cstr_alloc_islice_buffer(&sa, x.len);
    assert(ok);

    enum cstr_errcodes err;
    ok = cstr_skew(sa, x, &alpha, &err);
    assert(ok);

    for (int i = 0; i < sa.len; i++) {
        print_rotation(sa.len, x.buf, sa.buf[i]);
    }
    printf("\n");

    char* b = cstr_bwt(x.len, x.buf, sa.buf);
    for (int i = 0; i < sa.len; i++) {
        putchar(b[i] ? b[i] : '$');
    }
    putchar('\n');

    struct cstr_bwt_c_table* ctab = cstr_compute_bwt_c_table(x.len, b, 256);
    struct cstr_bwt_o_table* otab = cstr_compute_bwt_o_table(x.len, b, ctab);
    int left, right;
    cstr_bwt_search(&left, &right, x.buf, "is", ctab, otab);
    printf("[%d,%d]\n", left, right);
    for (int i = left; i < right; i++) {
        printf("%s\n", x.buf + sa.buf[i]);
    }

    free(otab);
    free(ctab);
    free(b);
    cstr_free_islice_buffer(&sa);

    return 0;
}
