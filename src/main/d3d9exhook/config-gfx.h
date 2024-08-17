#ifndef D3D9EXHOOK_CONFIG_GFX_H
#define D3D9EXHOOK_CONFIG_GFX_H

#include "iface-core/config.h"

/**
 * Config struct for d3d9exhook
 *
 * Note: forced_refresh_rate sets the monitor's refresh rate
 * (it does not limit FPS or anything)
 */
struct d3d9exhook_config_gfx {
    bool framed;
    bool windowed;
    bool confined;
    int32_t window_width;
    int32_t window_height;
    int32_t window_x;
    int32_t window_y;
    int32_t forced_refresh_rate;
    int32_t device_adapter;
    int32_t force_orientation;

    struct force_screen_res {
        int32_t width;
        int32_t height;
    } force_screen_res;
};

void d3d9exhook_config_gfx_get(
    const bt_core_config_t *config,
    struct d3d9exhook_config_gfx *config_gfx);

#endif
