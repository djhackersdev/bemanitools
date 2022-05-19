#ifndef DDRHOOK_UTIL_GFX_H
#define DDRHOOK_UTIL_GFX_H

#include <windows.h>

#include <stdbool.h>

void gfx_insert_hooks(HMODULE target);
bool gfx_get_windowed(void);
void gfx_set_windowed(void);

#endif
