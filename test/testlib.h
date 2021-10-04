#ifndef TESTLIB_H
#define TESTLIB_H

/*
WARNING:
This testing library is unsafe as hell. I'm using
underscore names, that might clash with system libraries,
I'm using longjmp to escape tests, that will almost
surely leak memory, and I use a lot of macros that
inject variables into functions. It is *not* something
you should use in production code. It is just something
quick and dirty for testing, and certainly nothing more.
*/

#include <setjmp.h>
#include <stdio.h>

struct tl_state
{
    int no_tests;
    int no_errors;
    jmp_buf escape;
};

void tl_error(struct tl_state *state, char *msg);
void tl_fatal(struct tl_state *state, char *msg);

#define TL_FORMAT_ERR(ERRFN, STATE, FMT, ...)  \
    do                                         \
    {                                          \
        char buf[1024];                        \
        snprintf(buf, 1024, FMT, __VA_ARGS__); \
        (ERRFN)((STATE), buf);                 \
    } while (0)

#define TL_DO_TEST(EXPR, ERRFN, STATE, FMT, ...)             \
    do                                                       \
    {                                                        \
        _tl_state_.no_tests++;                               \
        if (EXPR)                                            \
        {                                                    \
            TL_FORMAT_ERR(ERRFN, (STATE), FMT, __VA_ARGS__); \
        }                                                    \
    } while (0)

#define TL_ERROR_IF(EXPR)                                                    \
    TL_DO_TEST(EXPR, tl_error, &_tl_state_, "error: %s(%d): %s\n", __FILE__, \
               __LINE__, #EXPR);

#define TL_FATAL_IF(EXPR)                                                    \
    TL_DO_TEST(EXPR, tl_fatal, &_tl_state_, "fatal: %s(%d): %s\n", __FILE__, \
               __LINE__, #EXPR);

#define TL_ERROR_IF_NEQ_INT(A, B)                                     \
    if ((A) != (B))                                                   \
    TL_FORMAT_ERR(tl_error, &_tl_state_, "error: %s(%d): %d != %d\n", \
                  __FILE__, __LINE__, A, B)

#define TL_TEST_PROTOTYPE(NAME) bool NAME(void);

#define TL_TEST(NAME) bool NAME(void)

#define TL_BEGIN()                                 \
    struct tl_state _tl_state_ = {.no_errors = 0}; \
    if (setjmp(_tl_state_.escape))                 \
    {                                              \
        goto _tl_escape_;                          \
    }

#define TL_END()                                                        \
    if (_tl_state_.no_errors == 0)                                      \
    {                                                                   \
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

#define TL_BEGIN_TEST_SUITE() struct tl_state _tl_state_ = {.no_errors = 0};

#define TL_RUN_TEST(FUNC)           \
    do                              \
    {                               \
        _tl_state_.no_tests++;      \
        if (FUNC() != true)         \
            _tl_state_.no_errors++; \
    } while (0)

#define TL_END_SUITE()                                                \
    if (_tl_state_.no_errors == 0)                                    \
    {                                                                 \
        fprintf(stderr, "%s completed witout errors.\n", __func__);   \
        return 0; /* success */                                       \
    }                                                                 \
    else                                                              \
    {                                                                 \
        fprintf(stderr, "%d out of %d tests failed in %s.\n",         \
                _tl_state_.no_errors, _tl_state_.no_tests, __func__); \
        return 1;                                                     \
    }

int tl_test_array(void *restrict expected, void *restrict actual, size_t arrlen,
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

#define _TL_HANDLE_ARRAY_GENERIC(EXPECTED, ACTUAL, IDX)                       \
    TL_FORMAT_ERR(tl_error, &_tl_state_, "error: %s(%d): %s[%d] != %s[%d]\n", \
                  __FILE__, __LINE__, #EXPECTED, IDX, #ACTUAL, IDX)
#define _TL_HANDLE_ARRAY_GENERIC_FATAL(EXPECTED, ACTUAL, IDX)                 \
    TL_FORMAT_ERR(tl_fatal, &_tl_state_, "fatal: %s(%d): %s[%d] != %s[%d]\n", \
                  __FILE__, __LINE__, #EXPECTED, IDX, #ACTUAL, IDX)

#define TL_TEST_EQUAL_ARRAYS(EXPECTED, ACTUAL, LEN) \
    _TL_TEST_EQUAL_ARRAYS(EXPECTED, ACTUAL, LEN, _TL_HANDLE_ARRAY_GENERIC)
#define TL_TEST_EQUAL_ARRAYS_FATAL(EXPECTED, ACTUAL, LEN) \
    _TL_TEST_EQUAL_ARRAYS(EXPECTED, ACTUAL, LEN, _TL_HANDLE_ARRAY_GENERIC_FATAL)

#define _TL_HANDLE_ARRAY_INT(EXPECTED, ACTUAL, IDX)                          \
    TL_FORMAT_ERR(tl_error, &_tl_state_,                                     \
                  "error: %s(%d): %s[%d] (%d) != %s[%d] (%d)\n", __FILE__,   \
                  __LINE__, #EXPECTED, IDX, (EXPECTED)[(IDX)], #ACTUAL, IDX, \
                  (ACTUAL)[(IDX)])
#define _TL_HANDLE_ARRAY_INT_FATAL(EXPECTED, ACTUAL, IDX)                    \
    TL_FORMAT_ERR(tl_fatal, &_tl_state_,                                     \
                  "fatal: %s(%d): %s[%d] (%d) != %s[%d] (%d)\n", __FILE__,   \
                  __LINE__, #EXPECTED, IDX, (EXPECTED)[(IDX)], #ACTUAL, IDX, \
                  (ACTUAL)[(IDX)])

#define TL_TEST_EQUAL_INT_ARRAYS(EXPECTED, ACTUAL, LEN) \
    _TL_TEST_EQUAL_ARRAYS(EXPECTED, ACTUAL, LEN, _TL_HANDLE_ARRAY_INT)
#define TL_TEST_EQUAL_INT_ARRAYS_FATAL(EXPECTED, ACTUAL, LEN) \
    _TL_TEST_EQUAL_ARRAYS(EXPECTED, ACTUAL, LEN, _TL_HANDLE_ARRAY_INT_FATAL)

// Generating test strings
void tl_random_string(const char *alpha, int alpha_size, char *buf,
                      int buf_len);

#endif // TESTLIB_H
