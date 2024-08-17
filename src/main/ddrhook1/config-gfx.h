#ifndef DDRHOOK1_CONFIG_GFX_H
#define DDRHOOK1_CONFIG_GFX_H

#include <stdbool.h>

#include "api/core/config.h"

typedef struct ddrhook1_config_gfx {
    bool windowed;
} ddrhook1_config_gfx_t;

void ddrhook1_config_gfx_get(
    const bt_core_config_t *config,
    ddrhook1_config_gfx_t *config_gfx);

#endif
