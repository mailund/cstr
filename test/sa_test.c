#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>
#include <unity.h>

void setUp() {}
void tearDown() {}

void check_suffix_ordered(char const *x, int n, int sa[n]) {
    for (int i = 1; i < n; i++) {
        TEST_ASSERT(strcmp(x + sa[i - 1], x + sa[i]) < 0);
    }
}

void test_mississippi() {
    char const *x = "mississippi";
    int n = strlen(x);
    int *sa = cstr_skew(x);
    check_suffix_ordered(x, n + 1, sa);
    free(sa);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_mississippi);
    return UNITY_END();
}
