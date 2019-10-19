#ifndef TEST_CHECK_H
#define TEST_CHECK_H

#include <stdbool.h>
#include <string.h>

#define check_char_eq(expr, expected)                                    \
    {                                                                    \
        char _result = expr;                                             \
                                                                         \
        if (_result != expected) {                                       \
            check_char_eq_failed(                                        \
                __FILE__, __LINE__, __func__, #expr, _result, expected); \
        }                                                                \
    }

#define check_char_neq(expr, expected)                                   \
    {                                                                    \
        char _result = expr;                                             \
                                                                         \
        if (_result == expected) {                                       \
            check_char_neq_failed(                                       \
                __FILE__, __LINE__, __func__, #expr, _result, expected); \
        }                                                                \
    }

#define check_int_eq(expr, expected)                                     \
    {                                                                    \
        int _result = expr;                                              \
                                                                         \
        if (_result != expected) {                                       \
            check_int_eq_failed(                                         \
                __FILE__, __LINE__, __func__, #expr, _result, expected); \
        }                                                                \
    }

#define check_int_neq(expr, expected)                                    \
    {                                                                    \
        int _result = expr;                                              \
                                                                         \
        if (_result == expected) {                                       \
            check_int_neq_failed(                                        \
                __FILE__, __LINE__, __func__, #expr, _result, expected); \
        }                                                                \
    }

#define check_float_eq(expr, expected, delta) \
    {                                         \
        float _result = expr;                 \
                                              \
        if (_result - expected > delta) {     \
            check_float_eq_failed(            \
                __FILE__,                     \
                __LINE__,                     \
                __func__,                     \
                #expr,                        \
                _result,                      \
                expected,                     \
                delta);                       \
        }                                     \
    }

#define check_bool_true(expr)                                        \
    {                                                                \
        bool _result = expr;                                         \
                                                                     \
        if (!_result) {                                              \
            check_bool_eq_failed(                                    \
                __FILE__, __LINE__, __func__, #expr, _result, true); \
        }                                                            \
    }

#define check_bool_false(expr)                                        \
    {                                                                 \
        bool _result = expr;                                          \
                                                                      \
        if (_result) {                                                \
            check_bool_eq_failed(                                     \
                __FILE__, __LINE__, __func__, #expr, _result, false); \
        }                                                             \
    }

#define check_str_eq(expr, expected)                                     \
    {                                                                    \
        const char *_result = expr;                                      \
                                                                         \
        if (strcmp(_result, expected) != 0) {                            \
            check_str_eq_failed(                                         \
                __FILE__, __LINE__, __func__, #expr, _result, expected); \
        }                                                                \
    }

#define check_data_eq(expr, result_size, expected, expected_size) \
    {                                                             \
        const void *_result = expr;                               \
                                                                  \
        if (result_size != expected_size ||                       \
            memcmp(expr, expected, expected_size)) {              \
            check_data_eq_failed(                                 \
                __FILE__,                                         \
                __LINE__,                                         \
                __func__,                                         \
                #expr,                                            \
                _result,                                          \
                expected,                                         \
                result_size,                                      \
                expected_size);                                   \
        }                                                         \
    }

#define check_non_null(expr)                                            \
    {                                                                   \
        if (!expr) {                                                    \
            check_non_null_failed(__FILE__, __LINE__, __func__, #expr); \
        }                                                               \
    }

#define check_null(expr)                                            \
    {                                                               \
        if (expr) {                                                 \
            check_null_failed(__FILE__, __LINE__, __func__, #expr); \
        }                                                           \
    }

#define check_fail()                                    \
    {                                                   \
        check_failed(__FILE__, __LINE__, __func__, ""); \
    }

#define check_fail_msg(expr)                              \
    {                                                     \
        check_failed(__FILE__, __LINE__, __func__, expr); \
    }

void _Noreturn check_char_eq_failed(
    const char *file,
    int line,
    const char *func,
    char *expr,
    char result,
    char expected);
void _Noreturn check_char_neq_failed(
    const char *file,
    int line,
    const char *func,
    char *expr,
    char result,
    char expected);
void _Noreturn check_int_eq_failed(
    const char *file,
    int line,
    const char *func,
    char *expr,
    int result,
    int expected);
void _Noreturn check_int_neq_failed(
    const char *file,
    int line,
    const char *func,
    char *expr,
    int result,
    int expected);
void _Noreturn check_bool_eq_failed(
    const char *file,
    int line,
    const char *func,
    char *expr,
    bool result,
    bool expected);
void _Noreturn check_float_eq_failed(
    const char *file,
    int line,
    const char *func,
    char *expr,
    float result,
    float expected,
    float delta);
void _Noreturn check_str_eq_failed(
    const char *file,
    int line,
    const char *func,
    char *expr,
    const char *result,
    const char *expected);
void _Noreturn check_data_eq_failed(
    const char *file,
    int line,
    const char *func,
    char *expr,
    const void *result,
    const void *expected,
    size_t expr_size,
    size_t expected_size);
void _Noreturn check_non_null_failed(
    const char *file, int line, const char *func, char *expr);
void _Noreturn check_null_failed(
    const char *file, int line, const char *func, char *expr);
void _Noreturn check_failed(
    const char *file, int line, const char *func, char *expr);

#endif