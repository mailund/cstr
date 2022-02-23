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

#endif // GEN_UNIT_TESTS
#endif // CSTR_UNITTESTS_H
