#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>

#include "testlib.h"

typedef cstr_exact_matcher *(*algorithm_fn)(cstr_const_sslice, cstr_const_sslice);
#define NEXT cstr_exact_next_match
#define END -1

static TL_PARAM_TEST(test_simple_cases_p, algorithm_fn f)
{
    TL_BEGIN();
    {
        cstr_const_sslice x = CSTR_SLICE_STRING0((const char *)"aaba");
        cstr_const_sslice p = CSTR_SLICE_STRING((const char *)"a");
        cstr_bit_vector *expected = cstr_new_bv_from_string("11010");
        cstr_bit_vector *observed = cstr_new_bv_init(x.len);
        
        cstr_exact_matcher *m = f(x, p);
        for (long long i = NEXT(m); i != END; i = NEXT(m))
        {
            cstr_bv_set(observed, i, true);
        }
        cstr_free_exact_matcher(m);
        
        cstr_bv_print(expected);
        cstr_bv_print(observed);
        assert(cstr_bv_eq(expected, observed));
        
        free(expected);
        free(observed);
    }
    {
        cstr_const_sslice x = CSTR_SLICE_STRING0((const char *)"abab");
        cstr_const_sslice p = CSTR_SLICE_STRING((const char *)"ab");
        cstr_bit_vector *expected = cstr_new_bv_from_string("10100");
        cstr_bit_vector *observed = cstr_new_bv_init(x.len);

        cstr_exact_matcher *m = f(x, p);
        for (long long i = NEXT(m); i != END; i = NEXT(m))
        {
            cstr_bv_set(observed, i, true);
        }
        cstr_free_exact_matcher(m);
        
        assert(cstr_bv_eq(expected, observed));
        free(expected);
        free(observed);
    }
    {
        cstr_const_sslice x = CSTR_SLICE_STRING0((const char *)"aaaa");
        cstr_const_sslice p = CSTR_SLICE_STRING((const char *)"aa");
        cstr_bit_vector *expected = cstr_new_bv_from_string("11100");
        cstr_bit_vector *observed = cstr_new_bv_init(x.len);

        cstr_exact_matcher *m = f(x, p);
        for (long long i = NEXT(m); i != END; i = NEXT(m))
        {
            cstr_bv_set(observed, i, true);
        }
        cstr_free_exact_matcher(m);
        
        assert(cstr_bv_eq(expected, observed));
        free(expected);
        free(observed);
    }
    TL_END();
}

static TL_PARAM_TEST(test_random_string_p, algorithm_fn f)
{
    TL_BEGIN();

    cstr_sslice *x_buf = cstr_alloc_sslice(100);
    x_buf->buf[x_buf->len - 1] = 0; // Sentinel
    
    // Search in x, but only sample random strings into
    // the prefix that doesn't include the sentinel
    cstr_sslice x = *x_buf;
    cstr_sslice x_sample = CSTR_PREFIX(x, -1);
    
    cstr_sslice *p_buf = cstr_alloc_sslice(5);
    cstr_sslice p = *p_buf;

    
    for (int i = 0; i < 10; i++)
    {
        tl_random_string(x_sample, (const uint8_t *)"abc", 3);
        for (int j = 0; j < 10; j++)
        {
            tl_random_string(p, (const uint8_t *)"abc", 3);

            cstr_exact_matcher *matcher = f(CSTR_SLICE_CONST_CAST(x), CSTR_SLICE_CONST_CAST(p));
            for (long long m = cstr_exact_next_match(matcher);
                 m != -1;
                 m = cstr_exact_next_match(matcher))
            {
                cstr_sslice match = CSTR_SUBSLICE(x, m, m + p.len);
                TL_ERROR_IF(!CSTR_SLICE_EQ(match, p));
            }
            cstr_free_exact_matcher(matcher);
        }
    }

    free(p_buf);
    free(x_buf);

    TL_END();
}

static TL_PARAM_TEST(test_prefix_p, algorithm_fn f)
{
    TL_BEGIN();

    cstr_sslice *x_buf = cstr_alloc_sslice(100);
    x_buf->buf[x_buf->len - 1] = 0; // Sentinel
    
    // Search in x, but only sample random strings into
    // the prefix that doesn't include the sentinel
    cstr_sslice x = *x_buf;
    cstr_sslice x_sample = CSTR_PREFIX(x, -1);


    for (int k = 0; k < 10; k++)
    {
        tl_random_string(x_sample, (uint8_t const *)"abc", 3);
        for (int j = 0; j < 10; j++)
        {
            cstr_sslice p = tl_random_prefix(x);
            bool seen_0 = false;
            cstr_exact_matcher *m = f(CSTR_SLICE_CONST_CAST(x), CSTR_SLICE_CONST_CAST(p));
            for (long long i = NEXT(m); i != -1; i = NEXT(m))
            {
                if (i == 0LL)
                {
                    seen_0 = true;
                }
            }
            cstr_free_exact_matcher(m);
            TL_ERROR_IF(!seen_0);
        }
    }

    free(x_buf);

    TL_END();
}

static TL_PARAM_TEST(test_suffix_p, algorithm_fn f)
{
    TL_BEGIN();

    cstr_sslice *x_buf = cstr_alloc_sslice(100);
    x_buf->buf[x_buf->len - 1] = 0; // Sentinel
    
    // Search in x, but only sample random strings into
    // the prefix that doesn't include the sentinel,
    // and only take suffixes that do not include the
    // sentinel
    cstr_sslice x = *x_buf;
    cstr_sslice x_sample = CSTR_PREFIX(x, -1);


    for (int k = 0; k < 10; k++)
    {
        tl_random_string(x_sample, (uint8_t const *)"abc", 3);
        for (int j = 0; j < 10; j++)
        {
            cstr_sslice p = tl_random_suffix(x_sample);
            long long last_match = x.len - p.len - 1; // - 1 for sentinel
            bool seen_last = false;
            cstr_exact_matcher *m = f(CSTR_SLICE_CONST_CAST(x), CSTR_SLICE_CONST_CAST(p));
            for (long long i = NEXT(m); i != -1; i = NEXT(m))
            {
                if (i == last_match)
                {
                    seen_last = true;
                }
            }
            cstr_free_exact_matcher(m);
            TL_ERROR_IF(!seen_last);
        }
    }

    free(x_buf);

    TL_END();
}

// There is a lot of stuff we need to wrap up so we can free it again
// when we are done searching...
struct st_matcher
{
    cstr_exact_matcher matcher; // For the vtable
    cstr_alphabet *alpha;       // these are
    cstr_sslice *x_buf;         // all just here
    cstr_suffix_tree *st;       // so we can free them
    cstr_exact_matcher *m;      // the actual search
};

static long long st_next(struct st_matcher *m)
{
    return cstr_exact_next_match(m->m);
}
static void st_free(struct st_matcher *m)
{
    free(m->alpha);
    free(m->x_buf);
    cstr_free_suffix_tree(m->st);
    cstr_free_exact_matcher(m->m);
    free(m);
}
typedef long long (*next_f)(cstr_exact_matcher *);
typedef void (*free_f)(cstr_exact_matcher *);
static cstr_exact_matcher_vtab st_matcher_vtab = {.next = (next_f)st_next, .free = (free_f)st_free};

static cstr_exact_matcher *naive_st_matcher(cstr_const_sslice x, cstr_const_sslice p)
{
    struct st_matcher *matcher = cstr_malloc(sizeof *matcher);
    matcher->matcher = (cstr_exact_matcher){ .vtab = &st_matcher_vtab };
    
    // Construct the alphabet
    matcher->alpha = cstr_malloc(sizeof *matcher->alpha);
    cstr_init_alphabet(matcher->alpha, x);

    // Remap the string
    matcher->x_buf = cstr_alloc_sslice(x.len);
    cstr_alphabet_map(*matcher->x_buf, x, matcher->alpha);
    
    // Build the tree
    matcher->st = cstr_naive_suffix_tree(matcher->alpha,
                                         CSTR_SLICE_CONST_CAST(*matcher->x_buf));
    
    // and finally, construct the real matcher
    matcher->m = cstr_st_exact_search_map(matcher->st, p);
    
    return (cstr_exact_matcher *)matcher;
}

static cstr_exact_matcher *mcc_st_matcher(cstr_const_sslice x, cstr_const_sslice p)
{
    struct st_matcher *matcher = cstr_malloc(sizeof *matcher);
    matcher->matcher = (cstr_exact_matcher){ .vtab = &st_matcher_vtab };
    
    // Construct the alphabet
    matcher->alpha = cstr_malloc(sizeof *matcher->alpha);
    cstr_init_alphabet(matcher->alpha, x);

    // Remap the string
    matcher->x_buf = cstr_alloc_sslice(x.len);
    cstr_alphabet_map(*matcher->x_buf, x, matcher->alpha);
    
    // Build the tree
    matcher->st = cstr_mccreight_suffix_tree(matcher->alpha, CSTR_SLICE_CONST_CAST(*matcher->x_buf));
    
    // and finally, construct the real matcher
    matcher->m = cstr_st_exact_search_map(matcher->st, p);
    
    return (cstr_exact_matcher *)matcher;
}


struct sa_matcher
{
    cstr_exact_matcher matcher;
    cstr_suffix_array *sa;
    cstr_exact_matcher *m;
};

static long long sa_next(struct sa_matcher *m)
{
    return cstr_exact_next_match(m->m);
}

static void sa_free(struct sa_matcher *m)
{
    free(m->sa);
    free(m);
}

static cstr_exact_matcher_vtab sa_matcher_vtab = {.next = (next_f)sa_next, .free = (free_f)sa_free};

static cstr_exact_matcher *sa_matcher(cstr_const_sslice x, cstr_const_sslice p)
{
    struct sa_matcher *m = cstr_malloc(sizeof *m);
    m->matcher = (cstr_exact_matcher){ .vtab = &sa_matcher_vtab };
    
    cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, x);
    
    cstr_uislice *u_buf = cstr_alloc_uislice(x.len);
    cstr_alphabet_map_to_uint(*u_buf, x, &alpha);
    cstr_const_uislice u = CSTR_SLICE_CONST_CAST(*u_buf);
    
    m->sa = cstr_alloc_uislice(x.len);
    cstr_sais(*m->sa, u, &alpha);
    
    m->m = cstr_sa_bsearch(*m->sa, x, p);
    
    free(u_buf);
    
    return (cstr_exact_matcher *)m;
}


struct bwt_matcher
{
    cstr_exact_matcher matcher;
    cstr_bwt_preproc *preproc;
    cstr_exact_matcher *m;
};

static long long bwt_next(struct bwt_matcher *m)
{
    return cstr_exact_next_match(m->m);
}

static void bwt_free(struct bwt_matcher *m)
{
    cstr_free_bwt_preproc(m->preproc);
    cstr_free_exact_matcher(m->m);
    free(m);
}

static cstr_exact_matcher_vtab bwt_matcher_vtab = {.next = (next_f)bwt_next, .free = (free_f)bwt_free};

static cstr_exact_matcher *bwt_matcher(cstr_const_sslice x, cstr_const_sslice p)
{
    struct bwt_matcher *m = cstr_malloc(sizeof *m);
    m->matcher.vtab = &bwt_matcher_vtab;
    m->preproc = cstr_bwt_preprocess(x);
    m->m = cstr_fmindex_search(m->preproc, p);
    return (cstr_exact_matcher *)m;
}

static TL_TEST(simple_test)
{
    TL_BEGIN();
    TL_RUN_PARAM_TEST(test_simple_cases_p, "naive", cstr_naive_matcher);
    TL_RUN_PARAM_TEST(test_simple_cases_p, "ba", cstr_ba_matcher);
    TL_RUN_PARAM_TEST(test_simple_cases_p, "kmp", cstr_kmp_matcher);
    TL_RUN_PARAM_TEST(test_simple_cases_p, "naive-st", naive_st_matcher);
    TL_RUN_PARAM_TEST(test_simple_cases_p, "mcc-st", mcc_st_matcher);
    TL_RUN_PARAM_TEST(test_simple_cases_p, "sa_bsearch", sa_matcher);
    TL_RUN_PARAM_TEST(test_simple_cases_p, "fmindex", bwt_matcher);
    TL_END();
}

static TL_TEST(test_random_string)
{
    TL_BEGIN();
    TL_RUN_PARAM_TEST(test_random_string_p, "naive", cstr_naive_matcher);
    TL_RUN_PARAM_TEST(test_random_string_p, "ba", cstr_ba_matcher);
    TL_RUN_PARAM_TEST(test_random_string_p, "kmp", cstr_kmp_matcher);
    TL_RUN_PARAM_TEST(test_random_string_p, "naive-st", naive_st_matcher);
    TL_RUN_PARAM_TEST(test_random_string_p, "mcc-st", mcc_st_matcher);
    TL_RUN_PARAM_TEST(test_random_string_p, "sa_bsearch", sa_matcher);
    TL_RUN_PARAM_TEST(test_random_string_p, "fmindex", bwt_matcher);
    TL_END();
}

static TL_TEST(test_prefix)
{
    TL_BEGIN();
    TL_RUN_PARAM_TEST(test_prefix_p, "naive", cstr_naive_matcher);
    TL_RUN_PARAM_TEST(test_prefix_p, "ba", cstr_ba_matcher);
    TL_RUN_PARAM_TEST(test_prefix_p, "kmp", cstr_kmp_matcher);
    TL_RUN_PARAM_TEST(test_prefix_p, "naive-st", naive_st_matcher);
    TL_RUN_PARAM_TEST(test_prefix_p, "mcc-st", mcc_st_matcher);
    TL_RUN_PARAM_TEST(test_prefix_p, "sa_bsearch", sa_matcher);
    TL_RUN_PARAM_TEST(test_prefix_p, "fmindex", bwt_matcher);
    TL_END();
}

static TL_TEST(test_suffix)
{
    TL_BEGIN();
    TL_RUN_PARAM_TEST(test_suffix_p, "naive", cstr_naive_matcher);
    TL_RUN_PARAM_TEST(test_suffix_p, "ba", cstr_ba_matcher);
    TL_RUN_PARAM_TEST(test_suffix_p, "kmp", cstr_kmp_matcher);
    TL_RUN_PARAM_TEST(test_suffix_p, "naive-st", naive_st_matcher);
    TL_RUN_PARAM_TEST(test_suffix_p, "mcc-st", mcc_st_matcher);
    TL_RUN_PARAM_TEST(test_suffix_p, "sa_bsearch", sa_matcher);
    TL_RUN_PARAM_TEST(test_suffix_p, "fmindex", bwt_matcher);
    TL_END();
}

int main(void)
{
    TL_BEGIN_TEST_SUITE("exact_test");
    TL_RUN_TEST(simple_test);
    TL_RUN_TEST(test_random_string);
    TL_RUN_TEST(test_prefix);
    TL_RUN_TEST(test_suffix);
    TL_END_SUITE();
}
