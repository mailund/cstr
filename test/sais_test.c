#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unittests.h"
#include <testlib.h>

int main(void)
{
    TL_BEGIN_TEST_SUITE("sais test");
    TL_RUN_TEST(buckets_mississippi);
    TL_RUN_TEST(sais_classify_sl_mississippi);
    TL_RUN_TEST(sais_classify_sl_random);
    TL_RUN_TEST(buckets_lms_mississippi);
    TL_END_SUITE();
}
