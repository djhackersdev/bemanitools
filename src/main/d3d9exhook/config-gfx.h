#ifndef D3D9EXHOOK_CONFIG_GFX_H
#define D3D9EXHOOK_CONFIG_GFX_H

#include "cconfig/cconfig.h"

struct d3d9exhook_config_gfx {
    bool framed;
    bool windowed;
    int32_t window_width;
    int32_t window_height;
    int32_t forced_refresh_rate;
};

void d3d9exhook_config_gfx_init(struct cconfig *config);

void d3d9exhook_config_gfx_get(
    struct d3d9exhook_config_gfx *config_gfx, struct cconfig *config);

#endif
