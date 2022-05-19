#ifndef DDRHOOK1_D3D9_H
#define DDRHOOK1_D3D9_H

#include <stdbool.h>
#include <stdint.h>

#include "hook/d3d9.h"

struct ddrhook1_d3d9_config {
    bool windowed;
};

bool gfx_get_windowed(void);
void gfx_set_windowed(void);

bool gfx_get_is_modern(void);
void gfx_set_is_modern(void);

void gfx_d3d9_calc_win_size_with_framed(
    HWND hwnd, DWORD x, DWORD y, DWORD width, DWORD height, LPWINDOWPOS wp);

void gfx_insert_hooks(HMODULE target);
HRESULT gfx_d3d9_irp_handler(struct hook_d3d9_irp *irp);

#endif
