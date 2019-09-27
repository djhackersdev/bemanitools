#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test/check.h"

#include "util/hex.h"

static void _Noreturn __attribute__((format (printf, 4, 5))) fail(
        const char *file, int line, const char *func, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    fprintf(stderr, "%s:%d: In %s:\n", file, line, func);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    abort();
}

void _Noreturn check_char_eq_failed(const char *file, int line,
        const char *func, char *expr, char result, char expected)
{
    fail(file, line, func,
            "\tIncorrect result: %s\n"
            "\tReturn value was %c\n"
            "\tExpected         %c\n\n",
            expr, result, expected);
}

void _Noreturn check_char_neq_failed(const char *file, int line,
        const char *func, char *expr, char result, char expected)
{
    fail(file, line, func,
            "\tIncorrect result: %s\n"
            "\tReturn value was %c\n"
            "\tNot expected         %c\n\n",
            expr, result, expected);
}

void _Noreturn check_int_eq_failed(const char *file, int line,
        const char *func, char *expr, int result, int expected)
{
    fail(file, line, func,
            "\tIncorrect result: %s\n"
            "\tReturn value was %i\n"
            "\tExpected         %i\n\n",
            expr, result, expected);
}

void _Noreturn check_int_neq_failed(const char *file, int line,
        const char *func, char *expr, int result, int expected)
{
    fail(file, line, func,
            "\tIncorrect result: %s\n"
            "\tReturn value was %i\n"
            "\tNot expected         %i\n\n",
            expr, result, expected);
}

void _Noreturn check_bool_eq_failed(const char *file, int line,
        const char *func, char *expr, bool result, bool expected)
{
    fail(file, line, func,
            "\tIncorrect result: %s\n"
            "\tReturn value was %s\n"
            "\tExpected         %s\n\n",
            expr, result ? "true" : "false", expected ? "true" : "false");
}

void _Noreturn check_bool_neq_failed(const char *file, int line,
        const char *func, char *expr, bool result, bool expected)
{
    fail(file, line, func,
            "\tIncorrect result: %s\n"
            "\tReturn value was %s\n"
            "\tNot expected         %s\n\n",
            expr, result ? "true" : "false", expected ? "true" : "false");
}

void _Noreturn check_float_eq_failed(const char *file, int line,
        const char *func, char *expr, float result, float expected, float delta)
{
    fail(file, line, func,
            "\tIncorrect result: %s\n"
            "\tReturn value was %f\n"
            "\tDelta value      %f\n"
            "\tExpected         %f\n\n",
            expr, result, delta, expected);
}

void _Noreturn check_str_eq_failed(const char *file, int line,
        const char *func, char *expr, const char *result,
        const char *expected)
{
    fail(file, line, func,
            "\tIncorrect result: %s\n"
            "\tReturn value was \"%s\"\n"
            "\tExpected         \"%s\"\n\n",
            expr, result, expected);
}

void _Noreturn check_data_eq_failed(const char *file, int line,
        const char *func, char *expr, const void *result, const void *expected,
        size_t result_size, size_t expected_size)
{
    char result_data[result_size * 2 + 1];
    char expected_data[expected_size * 2 + 1];

    hex_encode_uc(result, result_size, result_data, sizeof(result_data));
    hex_encode_uc(expected, expected_size, expected_data, 
        sizeof(expected_data));

    fail(file, line, func,
            "\tIncorrect result: %s\n"
            "\tReturn size was: %d\n"
            "\tExected size: %d\n"
            "\tReturn value was \"%s\"\n"
            "\tExpected         \"%s\"\n\n",
            expr, result_size, expected_size, result_data, expected_data);
}

void _Noreturn check_non_null_failed(const char *file, int line, const char *func,
        char *expr)
{
    fail(file, line, func,
            "\tValue is not non-null: %s\n"
            "\tExpected non-null value\n\n",
            expr);
}

void _Noreturn check_null_failed(const char *file, int line, const char *func,
        char *expr)
{
    fail(file, line, func,
            "\tValue is not null: %s\n"
            "\tExpected null value\n\n",
            expr);
}