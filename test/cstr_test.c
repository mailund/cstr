#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>
#include <unity.h>

void setUp() {}
void tearDown() {}

static void test_cstr_creation(void) {
    struct cstr *cstr = cstr_cstr_from_string("foobar");
    TEST_ASSERT_EQUAL_INT(1, cstr->rc.refcount);
    TEST_ASSERT_EQUAL_INT(6, cstr->len);
    TEST_ASSERT(strcmp(cstr->buf, "foobar") == 0);
    cstr_refcount_decref(cstr);
}

static void test_int_array_creation(void) {
    struct cstr_int_array *arr = cstr_alloc_int_array(10);
    TEST_ASSERT_EQUAL_INT(1, arr->rc.refcount);
    TEST_ASSERT_EQUAL_INT(10, arr->len);
    cstr_refcount_decref(arr);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_cstr_creation);
    RUN_TEST(test_int_array_creation);
    return UNITY_END();
}
