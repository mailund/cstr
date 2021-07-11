#ifndef UNITTESTS_H
#define UNITTESTS_H

#include "config.h" // do we generate unit tests?
#if GEN_UNIT_TESTS

#include <unity.h>

// skew.c
void skew_test_len_calc(void);

#endif // GEN_UNIT_TESTS
#endif // UNITTESTS_H