#ifndef TESTLIB_H
#define TESTLIB_H

#include <stdbool.h>
#include <stdio.h>

#include <cstr.h>

struct tl_state
{
    int no_tests;
    int no_errors;
};

#define TL_PRINT_ERR(FMT, ...)                                      \
    fprintf(stderr, "error: %s(%d): " FMT, /* Flawfinder: ignore */ \
            __FILE__, __LINE__, __VA_ARGS__);

#define TL_ERROR_IF_(EXPR, FMT, ...)        \
    do                                      \
    {                                       \
        _tl_state_.no_tests++;              \
        if (EXPR)                           \
        {                                   \
            _tl_state_.no_errors++;         \
            TL_PRINT_ERR(FMT, __VA_ARGS__); \
        }                                   \
    } while (0)
#define TL_FATAL_IF_(EXPR, FMT, ...)        \
    do                                      \
    {                                       \
        _tl_state_.no_tests++;              \
        if (EXPR)                           \
        {                                   \
            _tl_state_.no_errors++;         \
            TL_PRINT_ERR(FMT, __VA_ARGS__); \
            goto _tl_escape_;               \
        }                                   \
    } while (0)

// MARK: Testing expressions
#define TL_ERROR_IF(EXPR) \
    TL_ERROR_IF_(EXPR, "%s\n", #EXPR)
#define TL_FATAL_IF(EXPR) \
    TL_FATAL_IF_(EXPR, "%s\n", #EXPR)

// MARK: Testing primitive types
#define TL_ERROR_IF_EQ(FMT, A, B) \
    TL_ERROR_IF_((A) == (B), "(%s) " FMT " == (%s) " FMT "\n", #A, A, #B, B)
#define TL_FATAL_IF_EQ(FMT, A, B) \
    TL_FATAL_IF_((A) == (B), "(%s) " FMT " == (%s) " FMT "\n", #A, A, #B, B)
#define TL_ERROR_IF_NEQ(FMT, A, B) \
    TL_ERROR_IF_((A) != (B), "(%s) " FMT " != (%s) " FMT "\n", #A, A, #B, B)
#define TL_FATAL_IF_NEQ(FMT, A, B) \
    TL_FATAL_IF_((A) != (B), "(%s) " FMT " != (%s) " FMT "\n", #A, A, #B, B)

#define TL_ERROR_IF_EQ_INT(A, B) TL_ERROR_IF_EQ("%d", A, B)
#define TL_FATAL_IF_EQ_INT(A, B) TL_FATAL_IF_EQ("%d", A, B)
#define TL_ERROR_IF_NEQ_INT(A, B) TL_ERROR_IF_NEQ("%d", A, B)
#define TL_FATAL_IF_NEQ_INT(A, B) TL_FATAL_IF_NEQ("%d", A, B)

#define TL_ERROR_IF_EQ_UINT(A, B) TL_ERROR_IF_EQ("%u", A, B)
#define TL_FATAL_IF_EQ_UINT(A, B) TL_FATAL_IF_EQ("%u", A, B)
#define TL_ERROR_IF_NEQ_UINT(A, B) TL_ERROR_IF_NEQ("%u", A, B)
#define TL_FATAL_IF_NEQ_UINT(A, B) TL_FATAL_IF_NEQ("%u", A, B)

#define TL_ERROR_IF_EQ_LL(A, B) TL_ERROR_IF_EQ("%lld", A, B)
#define TL_FATAL_IF_EQ_LL(A, B) TL_FATAL_IF_EQ("%lld", A, B)
#define TL_ERROR_IF_NEQ_LL(A, B) TL_ERROR_IF_NEQ("%lld", A, B)
#define TL_FATAL_IF_NEQ_LL(A, B) TL_FATAL_IF_NEQ("%lld", A, B)

#define TL_ERROR_IF_EQ_SIZE_T(A, B) TL_ERROR_IF_EQ("%zu", A, B)
#define TL_FATAL_IF_EQ_SIZE_T(A, B) TL_FATAL_IF_EQ("%zu", A, B)
#define TL_ERROR_IF_NEQ_SIZE_T(A, B) TL_ERROR_IF_NEQ("%zu", A, B)
#define TL_FATAL_IF_NEQ_SIZE_T(A, B) TL_FATAL_IF_NEQ("%zu", A, B)

#define TL_ERROR_IF_EQ_STRING(A, B) \
    TL_ERROR_IF_(strcmp((const char *)(A), (const char *)(B)) == 0, "%s != %s\n", A, B)
#define TL_FATAL_IF_EQ_STRING(A, B) \
    TL_FATAL_IF_(strcmp((const char *)(A), (const char *)(B)) == 0, "%s != %s\n", A, B)

#define TL_ERROR_IF_NEQ_STRING(A, B) \
    TL_ERROR_IF_(strcmp((const char *)(A), (const char *)(B)) != 0, "%s != %s\n", A, B)
#define TL_FATAL_IF_NEQ_STRING(A, B) \
    TL_FATAL_IF_(strcmp((const char *)(A), (const char *)(B)) != 0, "%s != %s\n", A, B)

#define TL_ERROR_IF_GT_STRING(A, B) \
    TL_ERROR_IF_(strcmp((const char *)(A), (const char *)(B)) > 0, "%s > %s\n", A, B)
#define TL_FATAL_IF_GT_STRING(A, B) \
    TL_FATAL_IF_(strcmp((const char *)(A), (const char *)(B)) > 0, "%s > %s\n", A, B)

#define TL_ERROR_IF_GE_STRING(A, B) \
    TL_ERROR_IF_(strcmp((const char *)(A), (const char *)(B)) >= 0, "%s >= %s\n", A, B)
#define TL_FATAL_IF_GE_STRING(A, B) \
    TL_FATAL_IF_(strcmp((const char *)(A), (const char *)(B)) >= 0, "%s >= %s\n", A, B)

#define TL_ERROR_IF_LT_STRING(A, B) \
    TL_ERROR_IF_(strcmp((const char *)(A), (const char *)(B)) < 0, "%s < %s\n", A, B)
#define TL_FATAL_IF_LT_STRING(A, B) \
    TL_FATAL_IF_(strcmp((const char *)(A), (const char *)(B)) < 0, "%s < %s\n", A, B)

#define TL_ERROR_IF_LE_STRING(A, B) \
    TL_ERROR_IF_(strcmp((const char *)(A), (const char *)(B)) <= 0, "%s <= %s\n", A, B)
#define TL_FATAL_IF_LE_STRING(A, B) \
    TL_FATAL_IF_(strcmp((const char *)(A), (const char *)(B)) <= 0, "%s <= %s\n", A, B)

#define TL_ERROR_IF_NEQ_SLICE(A, B) \
    TL_ERROR_IF_(!CSTR_SLICE_EQ(A, B), "%s != %s\n", #A, #B)
#define TL_FATAL_IF_NEQ_SLICE(A, B) \
    TL_FATAL_IF_(!CSTR_SLICE_EQ(A, B), "%s != %s\n", #A, #B)

#define TL_ERROR_IF_GE_SLICE(A, B) \
    TL_ERROR_IF_(CSTR_SLICE_GE(A, B), "%s >= %s\n", #A, #B)
#define TL_FATAL_IF_GE_SLICE(A, B) \
    TL_FATAL_IF_(CSTR_SLICE_GE(A, B), "%s >= %s\n", #A, #B)
#define TL_ERROR_IF_LE_SLICE(A, B) \
    TL_ERROR_IF_(CSTR_SLICE_LE(A, B), "%s <= %s\n", #A, #B)
#define TL_FATAL_IF_LE_SLICE(A, B) \
    TL_FATAL_IF_(CSTR_SLICE_LE(A, B), "%s <= %s\n", #A, #B)

// MARK: Testing arrays
int tl_test_array(void *restrict expected,
                  void *restrict actual,
                  size_t arrlen,
                  size_t objsize);

#define TL_TEST_EQUAL_ARRAYS_(EXPECTED, ACTUAL, LEN, HANDLER)         \
    do                                                                \
    {                                                                 \
        _tl_state_.no_tests++;                                        \
        int err_idx =                                                 \
            tl_test_array(EXPECTED, ACTUAL, LEN, sizeof *(EXPECTED)); \
        if (err_idx != -1)                                            \
        {                                                             \
            HANDLER(EXPECTED, ACTUAL, err_idx);                       \
        }                                                             \
    } while (0)

#define TL_HANDLE_ARRAY_GENERIC_(EXPECTED, ACTUAL, IDX) \
    do                                                  \
    {                                                   \
        _tl_state_.no_errors++;                         \
        TL_PRINT_ERR("%s[%d] != %s[%d]\n",              \
                     #EXPECTED, IDX, #ACTUAL, IDX);     \
    } while (0)

#define TL_HANDLE_ARRAY_GENERIC_FATAL_(EXPECTED, ACTUAL, IDX) \
    do                                                        \
    {                                                         \
        _tl_state_.no_errors++;                               \
        TL_PRINT_ERR("%s[%d] != %s[%d]\n",                    \
                     #EXPECTED, IDX, #ACTUAL, IDX);           \
        goto _tl_escape_;                                     \
    } while (0)

#define TL_HANDLE_ARRAY_INT_(EXPECTED, ACTUAL, IDX)     \
    do                                                  \
    {                                                   \
        _tl_state_.no_errors++;                         \
        TL_PRINT_ERR("%s[%d] (%d) != %s[%d] (%d)\n",    \
                     #EXPECTED, IDX, (EXPECTED)[(IDX)], \
                     #ACTUAL, IDX, (ACTUAL)[(IDX)]);    \
    } while (0)

#define TL_HANDLE_ARRAY_INT_FATAL_(EXPECTED, ACTUAL, IDX) \
    do                                                    \
    {                                                     \
        _tl_state_.no_errors++;                           \
        TL_PRINT_ERR("%s[%d] (%d) != %s[%d] (%d)\n",      \
                     #EXPECTED, IDX, (EXPECTED)[(IDX)],   \
                     #ACTUAL, IDX, (ACTUAL)[(IDX)]);      \
        goto _tl_escape_;                                 \
    } while (0)

#define TL_TEST_EQUAL_ARRAYS(EXPECTED, ACTUAL, LEN) \
    TL_TEST_EQUAL_ARRAYS_(EXPECTED, ACTUAL, LEN,    \
                          TL_HANDLE_ARRAY_GENERIC_)

#define TL_TEST_EQUAL_ARRAYS_FATAL(EXPECTED, ACTUAL, LEN) \
    TL_TEST_EQUAL_ARRAYS_(EXPECTED, ACTUAL, LEN,          \
                          TL_HANDLE_ARRAY_GENERIC_FATAL_)

#define TL_TEST_EQUAL_INT_ARRAYS(EXPECTED, ACTUAL, LEN) \
    TL_TEST_EQUAL_ARRAYS_(EXPECTED, ACTUAL, LEN,        \
                          TL_HANDLE_ARRAY_GENERIC_)

#define TL_TEST_EQUAL_INT_ARRAYS_FATAL(EXPECTED, ACTUAL, LEN) \
    TL_TEST_EQUAL_ARRAYS_(EXPECTED, ACTUAL, LEN,              \
                          TL_HANDLE_ARRAY_GENERIC_FATAL_)

// MARK: Setting up test functions
// __VA_OPT__ is not standard C, so we need separate
// macros here...
#define TL_TEST(NAME) \
    bool NAME(const char *_tl_test_name)
#define TL_PARAM_TEST(NAME, ...) \
    bool NAME(const char *_tl_test_name, __VA_ARGS__)

#define TL_BEGIN()                         \
    printf("Running test %s\n", __func__); \
    struct tl_state _tl_state_ = {         \
        .no_tests = 0, .no_errors = 0};    \
    if (0)                                 \
        goto _tl_escape_; /* to avoid warnings */

#define TL_END()                                                             \
    if (_tl_state_.no_errors == 0)                                           \
    {                                                                        \
        fprintf(stderr, "%d tests passed in %s.\n",                          \
                _tl_state_.no_tests, _tl_test_name);                         \
        return true; /* success */                                           \
    }                                                                        \
    fprintf(stderr, "%d out of %d tests failed in %s.\n",                    \
            _tl_state_.no_errors, _tl_state_.no_tests, _tl_test_name);       \
    return false;                                                            \
    _tl_escape_:                                                             \
    fprintf(stderr, "aborting test %s after fatal error.\n", _tl_test_name); \
    return false;

#define TL_BEGIN_TEST_SUITE(NAME) \
    TL_BEGIN();                   \
    const char *_tl_suite_name_ = NAME

#define TL_RUN_TEST(FUNC)           \
    do                              \
    {                               \
        _tl_state_.no_tests++;      \
        if (FUNC(#FUNC) != true)    \
            _tl_state_.no_errors++; \
    } while (0)
#define TL_RUN_PARAM_TEST(FUNC, NAME, ...)                 \
    do                                                     \
    {                                                      \
        _tl_state_.no_tests++;                             \
        if (FUNC(#FUNC "[" NAME "]", __VA_ARGS__) != true) \
            _tl_state_.no_errors++;                        \
    } while (0)

#define TL_END_SUITE()                                       \
    if (_tl_state_.no_errors == 0)                           \
    {                                                        \
        fprintf(stderr, "%d tests passed in suite %s.\n",    \
                _tl_state_.no_tests, _tl_suite_name_);       \
        return 0; /* success */                              \
    }                                                        \
    fprintf(stderr, "%d out of %d tests failed in %s.\n",    \
            _tl_state_.no_errors, _tl_state_.no_tests,       \
            _tl_suite_name_);                                \
    return 1;                                                \
    _tl_escape_:                                             \
    fprintf(stderr, "aborting test %s after fatal error.\n", \
            _tl_suite_name_);                                \
    return 1;

// MARK: Generating test strings
void tl_random_string(cstr_sslice x, const uint8_t *alpha, int alpha_size);
void tl_random_string0(cstr_sslice x, const uint8_t *alpha, int alpha_size);

cstr_sslice tl_random_prefix(cstr_sslice x);
cstr_sslice tl_random_suffix(cstr_sslice x);

#endif // TESTLIB_H
