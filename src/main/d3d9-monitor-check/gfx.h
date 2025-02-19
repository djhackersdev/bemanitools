#ifndef D3D9_MONITOR_CHECK_GFX_H
#define D3D9_MONITOR_CHECK_GFX_H

#include <windows.h>

#include <d3d9.h>

#include <stdbool.h>
#include <stdint.h>

typedef struct gfx gfx_t;
struct IDirect3DDevice9;

typedef struct gfx_adapter_modes {
    D3DDISPLAYMODE modes[1024];
    uint32_t count;
} gfx_adapter_modes_t;

typedef struct gfx_info {
    char adapter_identifier[1024];
    uint32_t width;
    uint32_t height;
    uint32_t refresh_rate;
    bool windowed;
    bool vsync;
} gfx_info_t;

bool gfx_adapter_info_get(D3DADAPTER_IDENTIFIER9 *adapter);

bool gfx_adapter_modes_get(gfx_adapter_modes_t *modes);

bool gfx_init(uint32_t width,
    uint32_t height,
    uint32_t refresh_rate,
    bool windowed,
    bool vsync,
    gfx_t **gfx);

uint32_t gfx_width_get(gfx_t *gfx);

uint32_t gfx_height_get(gfx_t *gfx);

bool gfx_info_get(gfx_t *gfx, gfx_info_t *info);

IDirect3DDevice9 *gfx_device_get(gfx_t *gfx);

bool gfx_frame_begin(gfx_t *gfx);

void gfx_frame_end(gfx_t *gfx);

uint64_t gfx_last_frame_count_get(gfx_t *gfx);

uint64_t gfx_last_frame_time_us_get(gfx_t *gfx);

bool gfx_adpater_info_get(gfx_t *gfx);

void gfx_fini(gfx_t *gfx);

#endif
