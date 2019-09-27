#include <stdio.h>

#include "util/defs.h"
#include "util/str.h"
#include "util/winres.h"

void vrsprintf(char *dest, size_t nchars, HINSTANCE inst,
        unsigned int fmt_resource, va_list ap)
{
    char fmt_buffer[1024];

    LoadStringA(inst, fmt_resource, fmt_buffer, lengthof(fmt_buffer));
    str_vformat(dest, nchars, fmt_buffer, ap);
    dest[nchars - 1] = '\0';
}

void rsprintf(char *dest, size_t nchars, HINSTANCE inst,
        unsigned int fmt_resource, ...)
{
    va_list ap;

    va_start(ap, fmt_resource);
    vrsprintf(dest, nchars, inst, fmt_resource, ap);
    va_end(ap);
}

void vrswprintf(wchar_t *dest, size_t nchars, HINSTANCE inst,
        unsigned int fmt_resource, va_list ap)
{
    wchar_t fmt_buffer[1024];

    LoadStringW(inst, fmt_resource, fmt_buffer, lengthof(fmt_buffer));
    wstr_vformat(dest, nchars, fmt_buffer, ap);
    dest[nchars - 1] = L'\0';
}

void rswprintf(wchar_t *dest, size_t nchars, HINSTANCE inst,
        unsigned int fmt_resource, ...)
{
    va_list ap;

    va_start(ap, fmt_resource);
    vrswprintf(dest, nchars, inst, fmt_resource, ap);
    va_end(ap);
}

void resource_open(struct resource *res, HINSTANCE inst, const char *type,
        unsigned int ord)
{
    HRSRC h1;
    HGLOBAL h2;

    h1 = FindResourceA(inst, MAKEINTRESOURCE(ord), type);
    h2 = LoadResource(inst, h1);

    res->bytes = LockResource(h2);
    res->nbytes = SizeofResource(inst, h1);
    res->pos = 0;
}

size_t resource_fgets(struct resource *res, char *chars, size_t nchars)
{
    size_t remain;
    size_t i;
    char c;

    c = '\0';
    remain = res->nbytes - res->pos;

    for (i = 0 ; i < remain ; i++) {
        if (c == '\n') {
            break;
        }

        c = res->bytes[res->pos++];

        if (i < nchars - 1) {
            chars[i] = c;
        }
    }

    chars[min(i, nchars - 1)] = '\0';

    return i > 0;
}

