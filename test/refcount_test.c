#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>
#include <unity.h>

void setUp() {}
void tearDown() {}

struct test_type {
    struct cstr_refcount_object rc;
    bool is_freed;
};

static void test_cleanup(void *obj) {
    ((struct test_type *)obj)->is_freed = true;
}

struct cstr_refcount_type test_type = {.cleanup = test_cleanup};

static void test_init(struct test_type *obj) {
    cstr_refcount_object_init((void *)obj, &test_type);
    obj->is_freed = false;
}

static void test_refcount(void) {
    struct test_type obj;
    test_init(&obj);
    TEST_ASSERT_FALSE(obj.is_freed);
    TEST_ASSERT_EQUAL_INT(1, obj.rc.refcount);

    cstr_refcount_incref(&obj);
    TEST_ASSERT_FALSE(obj.is_freed);
    TEST_ASSERT_EQUAL_INT(2, obj.rc.refcount);

    void *other_ref = cstr_refcount_incref(&obj);
    TEST_ASSERT(other_ref != NULL);
    TEST_ASSERT_FALSE(obj.is_freed);
    TEST_ASSERT_EQUAL_INT(3, obj.rc.refcount);

    cstr_refcount_decref(other_ref);
    TEST_ASSERT_FALSE(obj.is_freed);
    TEST_ASSERT_EQUAL_INT(2, obj.rc.refcount);

    cstr_refcount_decref(other_ref);
    cstr_refcount_decref(&obj);

    TEST_ASSERT_TRUE(obj.is_freed);
    TEST_ASSERT_EQUAL_INT(0, obj.rc.refcount);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_refcount);
    return UNITY_END();
}
