#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr_internal.h>

void setUp() {}
void tearDown() {}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(skew_test_len_calc);
    return UNITY_END();
}
