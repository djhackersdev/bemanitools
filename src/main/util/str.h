#ifndef UTIL_STR_H
#define UTIL_STR_H

#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <wchar.h>

void str_cat(char *dest, size_t dnchars, const char *src);
void str_cpy(char *dest, size_t dnchars, const char *src);
char *str_dup(const char *src);
bool str_ends_with(const char *haystack, const char *needle);
bool str_eq(const char *lhs, const char *rhs);
size_t str_format(char *buf, size_t nchars, const char *fmt, ...);
void str_trim(char *str);
size_t str_vformat(char *buf, size_t nchars, const char *fmt, va_list ap);
wchar_t *str_widen(const char *src);

void wstr_cat(wchar_t *dest, size_t dnchars, const wchar_t *src);
void wstr_cpy(wchar_t *dest, size_t dnchars, const wchar_t *src);
wchar_t *wstr_dup(const wchar_t *src);
bool wstr_ends_with(const wchar_t *haystack, const wchar_t *needle);
bool wstr_eq(const wchar_t *lhs, const wchar_t *rhs);
size_t wstr_format(wchar_t *buf, size_t nchars, const wchar_t *fmt, ...);
bool wstr_narrow(const wchar_t *src, char **dest);
size_t wstr_vformat(wchar_t *buf, size_t nchars, const wchar_t *fmt,
        va_list ap);

#endif
