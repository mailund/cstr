#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>
#include <unity.h>

void setUp() {}
void tearDown() {}

static int no_freed;
struct test_type {
    struct cstr_refcount_object rc;
    bool is_freed;
};

static void test_cleanup(void *obj) {
    ((struct test_type *)obj)->is_freed = true;
    no_freed++;
}

struct cstr_refcount_type test_type = {.cleanup = test_cleanup};

static void test_init(struct test_type *obj) {
    cstr_refcount_object_init(&obj->rc, &test_type);
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
    TEST_ASSERT_EQUAL_INT(1, no_freed);
}

static void auto_free(void) {
    struct test_type obj; // need an actual object for decref to do anything...
    cstr_refcount_object_init(&obj.rc, &test_type);
    
    CSTR_AUTO_DECREF struct test_type  *p = &obj; // this should be decref'ed
}

static void test_auto_free(void) {
    int current = no_freed;
    auto_free();
    TEST_ASSERT_EQUAL_INT(current + 1, no_freed);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_refcount);
    RUN_TEST(test_auto_free);
    return UNITY_END();
}
