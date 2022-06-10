#ifndef POPNHOOK1_CONFIG_GFX_H
#define POPNHOOK1_CONFIG_GFX_H

#include "cconfig/cconfig.h"

#include "popnhook1/d3d9.h"

// see struct popnhook1_d3d9_config for more info
struct popnhook1_config_gfx {
    bool framed;
    bool windowed;
    int32_t window_width;
    int32_t window_height;
};

void popnhook1_config_gfx_init(struct cconfig *config);

void popnhook1_config_gfx_get(
    struct popnhook1_config_gfx *config_gfx, struct cconfig *config);

#endif