#include <windows.h>

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "util/mem.h"
#include "util/str.h"

bool str_ends_with(const char *haystack, const char *needle)
{
    size_t haystack_len;
    size_t needle_len;

    haystack_len = strlen(haystack);
    needle_len = strlen(needle);

    if (needle_len > haystack_len) {
        return false;
    }

    return !memcmp(&haystack[haystack_len - needle_len], needle, needle_len);
}

bool wstr_ends_with(const wchar_t *haystack, const wchar_t *needle)
{
    size_t haystack_len;
    size_t needle_len;

    haystack_len = wcslen(haystack);
    needle_len = wcslen(needle);

    if (needle_len > haystack_len) {
        return false;
    }

    return !memcmp(
        &haystack[haystack_len - needle_len],
        needle,
        needle_len * sizeof(wchar_t));
}

size_t str_format(char *buf, size_t nchars, const char *fmt, ...)
{
    va_list ap;
    size_t result;

    va_start(ap, fmt);
    result = str_vformat(buf, nchars, fmt, ap);
    va_end(ap);

    return result;
}

size_t str_vformat(char *buf, size_t nchars, const char *fmt, va_list ap)
{
    int result;

    result = _vsnprintf(buf, nchars, fmt, ap);

    if (result >= (int) nchars || result < 0) {
        abort();
    }

    return (size_t) result;
}

size_t wstr_format(wchar_t *buf, size_t nchars, const wchar_t *fmt, ...)
{
    va_list ap;
    size_t result;

    va_start(ap, fmt);
    result = wstr_vformat(buf, nchars, fmt, ap);
    va_end(ap);

    return result;
}

size_t wstr_vformat(wchar_t *buf, size_t nchars, const wchar_t *fmt, va_list ap)
{
    int result;

    result = _vsnwprintf(buf, nchars, fmt, ap);

    if (result >= (int) nchars || result < 0) {
        abort();
    }

    return (size_t) result;
}

void str_cat(char *dest, size_t dnchars, const char *src)
{
    size_t dlen;
    size_t slen;

    dlen = strlen(dest);
    slen = strlen(src);

    if (dlen + slen >= dnchars) {
        abort();
    }

    memcpy(dest + dlen, src, (slen + 1) * sizeof(char));
}

void str_cpy(char *dest, size_t dnchars, const char *src)
{
    size_t slen;

    slen = strlen(src);

    if (slen >= dnchars) {
        abort();
    }

    memcpy(dest, src, (slen + 1) * sizeof(char));
}

void wstr_cat(wchar_t *dest, size_t dnchars, const wchar_t *src)
{
    size_t dlen;
    size_t slen;

    dlen = wcslen(dest);
    slen = wcslen(src);

    if (dlen + slen >= dnchars) {
        abort();
    }

    memcpy(dest + dlen, src, (slen + 1) * sizeof(wchar_t));
}

void wstr_cpy(wchar_t *dest, size_t dnchars, const wchar_t *src)
{
    size_t slen;

    slen = wcslen(src);

    if (slen >= dnchars) {
        abort();
    }

    memcpy(dest, src, (slen + 1) * sizeof(wchar_t));
}

char *str_dup(const char *str)
{
    char *dest;
    size_t nbytes;

    nbytes = strlen(str) + 1;
    dest = xmalloc(nbytes);
    memcpy(dest, str, nbytes);

    return dest;
}

void str_trim(char *str)
{
    char *pos;

    for (pos = str + strlen(str) - 1; pos > str; pos--) {
        if (!isspace(*pos)) {
            return;
        }

        *pos = '\0';
    }
}

wchar_t *str_widen(const char *src)
{
    int nchars;
    wchar_t *result;

    nchars = MultiByteToWideChar(CP_ACP, 0, src, -1, NULL, 0);

    if (!nchars) {
        abort();
    }

    result = xmalloc(nchars * sizeof(wchar_t));

    if (!MultiByteToWideChar(CP_ACP, 0, src, -1, result, nchars)) {
        abort();
    }

    return result;
}

wchar_t *wstr_dup(const wchar_t *wstr)
{
    wchar_t *dest;
    size_t nchars;

    nchars = wcslen(wstr) + 1;
    dest = xmalloc(nchars * sizeof(wchar_t));
    memcpy(dest, wstr, nchars * sizeof(wchar_t));

    return dest;
}

bool wstr_narrow(const wchar_t *src, char **dest)
{
    int nbytes;

    nbytes = WideCharToMultiByte(CP_ACP, 0, src, -1, NULL, 0, NULL, NULL);

    if (nbytes <= 0) {
        goto size_fail;
    }

    *dest = xmalloc(nbytes);

    if (WideCharToMultiByte(CP_ACP, 0, src, -1, *dest, nbytes, NULL, NULL) !=
        nbytes) {
        goto conv_fail;
    }

    return true;

conv_fail:
    free(*dest);
    *dest = NULL;

size_fail:
    return false;
}

bool str_eq(const char *lhs, const char *rhs)
{
    if (lhs == NULL || rhs == NULL) {
        return lhs == rhs;
    } else {
        return strcmp(lhs, rhs) == 0;
    }
}

bool wstr_eq(const wchar_t *lhs, const wchar_t *rhs)
{
    if (lhs == NULL || rhs == NULL) {
        return lhs == rhs;
    } else {
        return wcscmp(lhs, rhs) == 0;
    }
}
