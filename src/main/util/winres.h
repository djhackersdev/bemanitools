#ifndef UTIL_WINRES_H
#define UTIL_WINRES_H

#include <windows.h>

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <wchar.h>

#ifdef _UNICODE
#define vrstprintf vrswprintf
#define rstprintf rswprintf
#else
#define vrstprintf vrsprintf
#define rstprintf rsprintf
#endif

struct resource {
    const char *bytes;
    size_t nbytes;
    size_t pos;
};

void vrsprintf(
    char *dest,
    size_t nchars,
    HINSTANCE inst,
    unsigned int fmt_resource,
    va_list ap);
void rsprintf(
    char *dest, size_t nchars, HINSTANCE inst, unsigned int fmt_resource, ...);

void vrswprintf(
    wchar_t *dest,
    size_t nchars,
    HINSTANCE inst,
    unsigned int fmt_resource,
    va_list ap);
void rswprintf(
    wchar_t *dest,
    size_t nchars,
    HINSTANCE inst,
    unsigned int fmt_resource,
    ...);

void resource_open(
    struct resource *res, HINSTANCE inst, const char *type, unsigned int ord);
size_t resource_fgets(struct resource *res, char *chars, size_t nchars);

#endif
