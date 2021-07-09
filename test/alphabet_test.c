#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>
#include <unity.h>

void setUp() {}
void tearDown() {}

static void test_create_alphabet(void) {
    char const *x = "foobar";
    struct cstr_alphabet *alpha = cstr_alphabet_from_string(x);

    TEST_ASSERT(alpha->map[0] == 0);
    TEST_ASSERT(alpha->map['a'] == 1);
    TEST_ASSERT(alpha->map['b'] == 2);
    TEST_ASSERT(alpha->map['f'] == 3);
    TEST_ASSERT(alpha->map['o'] == 4);
    TEST_ASSERT(alpha->map['r'] == 5);
    TEST_ASSERT_EQUAL(6, alpha->size);

    for (int i = 0; i < 256; i++) {
        if (alpha->map[i]) {
            TEST_ASSERT_EQUAL(alpha->revmap[alpha->map[i]], i);
        }
    }

    cstr_refcount_decref(alpha);
}

static void test_mapping(void) {
    char const *x = "foobar";
    struct cstr_alphabet *alpha = cstr_alphabet_from_string(x);

    struct cstr *mapped = cstr_alphabet_map(alpha, cstr_slice_from_string((char *)x));
    TEST_ASSERT(mapped != NULL);

    TEST_ASSERT(strcmp(mapped->buf, "\3\4\4\2\1\5") == 0);
    cstr_refcount_decref(mapped);

    mapped = cstr_alphabet_map(alpha, cstr_slice_from_string("qux"));
    TEST_ASSERT(mapped == NULL);

    cstr_refcount_decref(alpha);
}

static void test_int_mapping(void) {
    char const *x = "foobar";
    struct cstr_alphabet *alpha = cstr_alphabet_from_string(x);
    
    struct cstr_int_array *mapped = cstr_alphabet_map_to_int(alpha, cstr_slice_from_string((char *)x));
    TEST_ASSERT(mapped != NULL);
    
    int expected[] = { 3, 4, 4, 2, 1, 5, 0 };
    TEST_ASSERT_EQUAL_INT_ARRAY(expected, mapped->buf, sizeof(expected)/sizeof(*expected));
    cstr_refcount_decref(mapped);
    
    mapped = cstr_alphabet_map_to_int(alpha, cstr_slice_from_string("qux"));
    TEST_ASSERT(mapped == NULL);
    
    cstr_refcount_decref(alpha);
}


static void test_revmapping(void) {
    char const *x = "foobar";
    struct cstr_alphabet *alpha = cstr_alphabet_from_string(x);

    struct cstr *mapped = cstr_alphabet_map(alpha, cstr_slice_from_string((char *)x));
    struct cstr *rev = cstr_alphabet_revmap(alpha, cstr_slice_from_cstr(mapped));

    TEST_ASSERT(strcmp(x, rev->buf) == 0);

    cstr_refcount_decref(mapped);
    cstr_refcount_decref(rev);
    cstr_refcount_decref(alpha);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_create_alphabet);
    RUN_TEST(test_mapping);
    RUN_TEST(test_int_mapping);
    RUN_TEST(test_revmapping);
    return UNITY_END();
}
