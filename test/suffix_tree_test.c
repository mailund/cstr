#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unittests.h"
#include <testlib.h>

int main(void)
{
    TL_BEGIN_TEST_SUITE("suffix tree test");
    TL_RUN_TEST(st_constructing_leaves);
    TL_RUN_TEST(st_constructing_inner_nodes);
    TL_RUN_TEST(st_attempted_scans);
    TL_RUN_TEST(st_dft);
    TL_END_SUITE();
}
