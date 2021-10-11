#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <testlib.h>
#include "unittests.h"

int main(void) {
    TL_BEGIN_TEST_SUITE("skew test");
    TL_RUN_TEST(skew_test_len_calc);
    TL_END_SUITE();
    return 0;
}
