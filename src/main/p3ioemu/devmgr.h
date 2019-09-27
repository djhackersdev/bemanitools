#ifndef P3IO_DEVMGR_H
#define P3IO_DEVMGR_H

/* Not called setupapi.h so that we don't collide with the real setupapi.h */

#include <windows.h>

#include <stdbool.h>
#include <stddef.h>

void p3io_setupapi_insert_hooks(HMODULE target);
bool p3io_setupapi_match_path(const wchar_t *path);

#endif
