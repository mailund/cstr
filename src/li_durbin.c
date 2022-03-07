#include "bwt_internal.h"
#include "unittests.h"

struct cstr_li_durbin_preproc
{
    cstr_alphabet alpha;
    cstr_suffix_array *sa;
    struct c_table *ctab;
    struct o_table *otab;
    struct o_table *rotab;
};

void cstr_free_li_durbin_preproc(cstr_li_durbin_preproc *preproc)
{
    free(preproc->sa);
    free(preproc->ctab);
    free(preproc->otab);
    free(preproc->rotab);
    free(preproc);
}

cstr_li_durbin_preproc *
cstr_li_durbin_preprocess(cstr_const_sslice x)
{
    cstr_li_durbin_preproc *preproc = cstr_malloc(sizeof *preproc);

    cstr_init_alphabet(&preproc->alpha, x);

    // Map the string into an integer slice so we can build the suffix array.
    cstr_uislice *u_buf = cstr_alloc_uislice(x.len);
    cstr_alphabet_map_to_uint(*u_buf, x, &preproc->alpha);
    cstr_const_uislice u = CSTR_SLICE_CONST_CAST(*u_buf);

    cstr_sslice *w_buf = cstr_alloc_sslice(x.len); // Mapping x into w
    cstr_alphabet_map(*w_buf, x, &preproc->alpha);
    cstr_const_sslice w = CSTR_SLICE_CONST_CAST(*w_buf);

    preproc->sa = cstr_alloc_uislice(x.len); // Get the suffix array

    cstr_sslice *bwt_buf = cstr_alloc_sslice(x.len); // Get a buffer for the the bwt string
    cstr_const_sslice bwt = CSTR_SLICE_CONST_CAST(*bwt_buf);

    // Then build the suffix arrays. Build the reverse first,
    // so we don't need a copy for it later. We need to remember
    // the forward one, but not the backward one.
    // The PREFIX stuff is so we only reverse the prefix up to the sentinel,
    // but we do not include the sentinel in the reversal.
    CSTR_REV_SLICE(CSTR_PREFIX(*u_buf, -1)); // Both of these are representations of the input string.
    CSTR_REV_SLICE(CSTR_PREFIX(*w_buf, -1)); // Just with different types. We reverse to build RO

    cstr_sais(*preproc->sa, u, &preproc->alpha);
    cstr_bwt(*bwt_buf, w, *preproc->sa);
    preproc->ctab = cstr_build_c_table(bwt, preproc->alpha.size);
    preproc->rotab = cstr_build_o_table(bwt, preproc->ctab);

    // The C table is the same in either direction, but we need
    // to rebuild the suffix array and BWT to get the O table
    CSTR_REV_SLICE(CSTR_PREFIX(*u_buf, -1)); // Reverse the input back to the forward direction
    CSTR_REV_SLICE(CSTR_PREFIX(*w_buf, -1)); // so we can build the forward BWT and O table

    cstr_sais(*preproc->sa, u, &preproc->alpha);
    cstr_bwt(*bwt_buf, w, *preproc->sa);
    preproc->otab = cstr_build_o_table(bwt, preproc->ctab);

    // We don't need the mapped string nor the BWT any more.
    // The information we need is all in the tables in preproc.
    free(bwt_buf);
    free(w_buf);
    free(u_buf);

    return preproc;
}

#ifdef GEN_UNIT_TESTS // unit testing of static functions...

static void print_c_table(struct c_table const *ctab)
{
    printf("[ ");
    for (long long i = 0; i < ctab->sigma; i++)
    {
        printf("%u ", ctab->cumsum[i]);
    }
    printf("]\n");
}

static void print_o_table(struct o_table const *otab)
{
    for (int a = 0; a < otab->sigma; a++)
    {
        for (int i = 0; i < otab->n; i++)
        {
            printf("%lld ", O_RAW(a, i));
        }
        printf("\n");
    }
}

TL_TEST(build_ld_tables)
{
    TL_BEGIN();

    cstr_const_sslice x = CSTR_SLICE_STRING0((const char *)"mississippi");
    cstr_li_durbin_preproc *preproc = cstr_li_durbin_preprocess(x);
    printf("SA: ");
    CSTR_SLICE_PRINT(*preproc->sa);
    printf("\n");
    printf("C: ");
    print_c_table(preproc->ctab);
    printf("\n");
    printf("O:\n");
    print_o_table(preproc->otab);
    printf("\n");
    printf("RO:\n");
    print_o_table(preproc->rotab);

    cstr_free_li_durbin_preproc(preproc);

    TL_END();
}

#endif // GEN_UNIT_TESTS
