#ifndef JBHOOK1_CONFIG_GFX_H
#define JBHOOK1_CONFIG_GFX_H

#include "api/core/config.h"

/**
 * Struct holding configuration values for GFX related items.
 */
typedef struct jbhook1_config_gfx {
    bool windowed;
    bool vertical;
} jbhook1_config_gfx_t;

void jbhook1_config_gfx_get(
    const bt_core_config_t *config,
    jbhook1_config_gfx_t *config_out);

#endif
