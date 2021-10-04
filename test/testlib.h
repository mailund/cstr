#ifndef TESTLIB_H
#define TESTLIB_H

#include <stdio.h>

struct tl_state
{
    int no_tests;
    int no_errors;
};

#define TL_PRINT_ERR(FMT, ...)             \
    fprintf(stderr, "error: %s(%d): " FMT, \
            __FILE__, __LINE__, __VA_ARGS__);

#define _TL_ERROR_IF(EXPR, FMT, ...)        \
    do                                      \
    {                                       \
        _tl_state_.no_tests++;              \
        if (EXPR)                           \
        {                                   \
            _tl_state_.no_errors++;         \
            TL_PRINT_ERR(FMT, __VA_ARGS__); \
        }                                   \
    } while (0)

#define TL_ERROR_IF(EXPR) \
    _TL_ERROR_IF(EXPR, "%s\n", #EXPR)

#define TL_FATAL_IF(EXPR)                \
    do                                   \
    {                                    \
        _tl_state_.no_tests++;           \
        if (EXPR)                        \
        {                                \
            _tl_state_.no_errors++;      \
            TL_PRINT_ERR("%s\n", #EXPR); \
            goto _tl_escape_;            \
        }                                \
    } while (0)

#define TL_ERROR_IF_NEQ_INT(A, B) \
    _TL_ERROR_IF((A) != (B), "%d != %d\n", A, B)

#define TL_TEST_PROTOTYPE(NAME) bool NAME(void);
#define TL_TEST(NAME) bool NAME(void)

#define TL_BEGIN()                      \
    struct tl_state _tl_state_ = {      \
        .no_tests = 0, .no_errors = 0}; \
    if (0)                              \
        goto _tl_escape_; /* to avoid warnings */

#define TL_END()                                                        \
    if (_tl_state_.no_errors == 0)                                      \
    {                                                                   \
        fprintf(stderr, "%d tests passed in %s.\n",                     \
                _tl_state_.no_tests, __func__);                         \
        return true; /* success */                                      \
    }                                                                   \
    else                                                                \
    {                                                                   \
        fprintf(stderr, "%d out of %d tests failed in %s.\n",           \
                _tl_state_.no_errors, _tl_state_.no_tests, __func__);   \
        return false;                                                   \
    }                                                                   \
    _tl_escape_:                                                        \
    fprintf(stderr, "aborting test %s after fatal error.\n", __func__); \
    return false;

#define TL_BEGIN_TEST_SUITE() \
    TL_BEGIN()

#define TL_RUN_TEST(FUNC)           \
    do                              \
    {                               \
        _tl_state_.no_tests++;      \
        if (FUNC() != true)         \
            _tl_state_.no_errors++; \
    } while (0)

#define TL_END_SUITE()                                                  \
    if (_tl_state_.no_errors == 0)                                      \
    {                                                                   \
        fprintf(stderr, "%d tests passed in %s.\n",                     \
                _tl_state_.no_tests, __func__);                         \
        return 0; /* success */                                         \
    }                                                                   \
    else                                                                \
    {                                                                   \
        fprintf(stderr, "%d out of %d tests failed in %s.\n",           \
                _tl_state_.no_errors, _tl_state_.no_tests, __func__);   \
        return 1;                                                       \
    }                                                                   \
    _tl_escape_:                                                        \
    fprintf(stderr, "aborting test %s after fatal error.\n", __func__); \
    return 1;

int tl_test_array(void *restrict expected,
                  void *restrict actual,
                  size_t arrlen,
                  size_t objsize);

#define _TL_TEST_EQUAL_ARRAYS(EXPECTED, ACTUAL, LEN, HANDLER)         \
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

#define _TL_HANDLE_ARRAY_GENERIC(EXPECTED, ACTUAL, IDX) \
    do                                                  \
    {                                                   \
        _tl_state_.no_errors++;                         \
        TL_PRINT_ERR("%s[%d] != %s[%d]\n",              \
                     #EXPECTED, IDX, #ACTUAL, IDX);     \
    } while (0)

#define _TL_HANDLE_ARRAY_GENERIC_FATAL(EXPECTED, ACTUAL, IDX) \
    do                                                        \
    {                                                         \
        _tl_state_.no_errors++;                               \
        TL_PRINT_ERR("%s[%d] != %s[%d]\n",                    \
                     #EXPECTED, IDX, #ACTUAL, IDX);           \
        goto _tl_escape_;                                     \
    } while (0)

#define _TL_HANDLE_ARRAY_INT(EXPECTED, ACTUAL, IDX)     \
    do                                                  \
    {                                                   \
        _tl_state_.no_errors++;                         \
        TL_PRINT_ERR("%s[%d] (%d) != %s[%d] (%d)\n",    \
                     #EXPECTED, IDX, (EXPECTED)[(IDX)], \
                     #ACTUAL, IDX, (ACTUAL)[(IDX)]);    \
    } while (0)

#define _TL_HANDLE_ARRAY_INT_FATAL(EXPECTED, ACTUAL, IDX) \
    do                                                    \
    {                                                     \
        _tl_state_.no_errors++;                           \
        TL_PRINT_ERR("%s[%d] (%d) != %s[%d] (%d)\n",      \
                     #EXPECTED, IDX, (EXPECTED)[(IDX)],   \
                     #ACTUAL, IDX, (ACTUAL)[(IDX)]);      \
        goto _tl_escape_;                                 \
    } while (0)

#define TL_TEST_EQUAL_ARRAYS(EXPECTED, ACTUAL, LEN) \
    _TL_TEST_EQUAL_ARRAYS(EXPECTED, ACTUAL, LEN,    \
                          _TL_HANDLE_ARRAY_GENERIC)

#define TL_TEST_EQUAL_ARRAYS_FATAL(EXPECTED, ACTUAL, LEN) \
    _TL_TEST_EQUAL_ARRAYS(EXPECTED, ACTUAL, LEN,          \
                          _TL_HANDLE_ARRAY_GENERIC_FATAL)

#define TL_TEST_EQUAL_INT_ARRAYS(EXPECTED, ACTUAL, LEN) \
    _TL_TEST_EQUAL_ARRAYS(EXPECTED, ACTUAL, LEN,        \
                          _TL_HANDLE_ARRAY_GENERIC)

#define TL_TEST_EQUAL_INT_ARRAYS_FATAL(EXPECTED, ACTUAL, LEN) \
    _TL_TEST_EQUAL_ARRAYS(EXPECTED, ACTUAL, LEN,              \
                          _TL_HANDLE_ARRAY_GENERIC_FATAL)

// Generating test strings
void tl_random_string(const char *alpha, int alpha_size, char *buf,
                      int buf_len);

#endif // TESTLIB_H
