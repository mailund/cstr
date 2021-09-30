#ifndef UNITTESTS_H
#define UNITTESTS_H

#include "config.h" // do we generate unit tests?
#ifdef GEN_UNIT_TESTS

#include <testlib.h>

// skew.c
TL_TEST_PROTOTYPE(skew_test_len_calc);

#endif // GEN_UNIT_TESTS
#endif // UNITTESTS_H