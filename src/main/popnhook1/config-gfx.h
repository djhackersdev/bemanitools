#ifndef POPNHOOK1_CONFIG_GFX_H
#define POPNHOOK1_CONFIG_GFX_H

#include <stdbool.h>
#include <stdint.h>

#include "api/core/config.h"

typedef struct popnhook1_config_gfx {
    bool framed;
    bool windowed;
    int32_t window_width;
    int32_t window_height;
} popnhook1_config_gfx_t;

void popnhook1_config_gfx_get(
    const bt_core_config_t *config,
    popnhook1_config_gfx_t *out_config);
    
#endif