#ifndef UNITTESTS_H
#define UNITTESTS_H

#include "config.h" // do we generate unit tests?
#ifdef GEN_UNIT_TESTS

#include <testlib.h>

// skew.c
TL_TEST(skew_test_len_calc);

// sais.c
TL_TEST(buckets_mississippi);
TL_TEST(sais_classify_sl_mississippi);
TL_TEST(sais_classify_sl_random);
TL_TEST(buckets_lms_mississippi);
TL_TEST(induce_mississippi);

// suffix_tree.c
TL_TEST(st_constructing_leaves);
TL_TEST(st_constructing_inner_nodes);
TL_TEST(st_attempted_scans);
TL_TEST(st_dft);
TL_TEST(st_search);

// Li & Durbin
TL_TEST(build_ld_tables);
TL_TEST(ld_iterator);

#endif // GEN_UNIT_TESTS
#endif // CSTR_UNITTESTS_H
