#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>
#include <unity.h>

void setUp() {}
void tearDown() {}

static void test_create_alphabet(void) {
    enum cstr_errcodes err;
    
    char const *x = "foobar";
    struct cstr_alphabet *alpha = cstr_alphabet_from_string(x, &err);
    TEST_ASSERT(alpha);
    TEST_ASSERT_EQUAL(CSTR_NO_ERROR, err);

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

    free(alpha);
}

static void test_mapping(void) {
    enum cstr_errcodes err;
    
    char const *x = "foobar";
    struct cstr_alphabet *alpha = cstr_alphabet_from_string(x, &err);
    TEST_ASSERT_EQUAL(CSTR_NO_ERROR, err);

    char *mapped = cstr_alphabet_map(alpha, cstr_slice_from_string((char *)x), &err);
    TEST_ASSERT(mapped != NULL);
    TEST_ASSERT_EQUAL(CSTR_NO_ERROR, err);

    TEST_ASSERT(strcmp(mapped, "\3\4\4\2\1\5") == 0);
    free(mapped);
    mapped = 0;

    mapped = cstr_alphabet_map(alpha, cstr_slice_from_string("qux"), &err);
    TEST_ASSERT(mapped == NULL);
    TEST_ASSERT_EQUAL(CSTR_MAPPING_ERROR, err);

    free(alpha);
}

static void test_int_mapping(void) {
    enum cstr_errcodes err;
    
    char const *x = "foobar";
    struct cstr_alphabet *alpha = cstr_alphabet_from_string(x, &err);
    TEST_ASSERT_EQUAL(CSTR_NO_ERROR, err);

    unsigned int *mapped =
        cstr_alphabet_map_to_int(alpha, cstr_slice_from_string((char *)x), &err);
    TEST_ASSERT(mapped != NULL);
    TEST_ASSERT_EQUAL(CSTR_NO_ERROR, err);

    int expected[] = {3, 4, 4, 2, 1, 5, 0};
    TEST_ASSERT_EQUAL_INT_ARRAY(expected, mapped,
                                sizeof(expected) / sizeof(*expected));
    free(mapped);
    mapped = 0;

    mapped = cstr_alphabet_map_to_int(alpha, cstr_slice_from_string("qux"), &err);
    TEST_ASSERT(mapped == NULL);
    TEST_ASSERT_EQUAL(CSTR_MAPPING_ERROR, err);

    free(alpha);
}

static void test_revmapping(void) {
    enum cstr_errcodes err;
    
    char const *x = "foobar";
    struct cstr_alphabet *alpha = cstr_alphabet_from_string(x, &err);
    assert(alpha);

    char *mapped = cstr_alphabet_map(alpha, cstr_slice_from_string((char *)x), &err);
    char *rev = cstr_alphabet_revmap(alpha, cstr_slice_from_string(mapped), &err);
    
    TEST_ASSERT_EQUAL(CSTR_NO_ERROR, err);
    TEST_ASSERT(strcmp(x, rev) == 0);

    free(rev);
    free(mapped);
    free(alpha);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_create_alphabet);
    RUN_TEST(test_mapping);
    RUN_TEST(test_int_mapping);
    RUN_TEST(test_revmapping);
    return UNITY_END();
}
