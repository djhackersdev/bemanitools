#ifndef DDRHOOKX_CONFIG_GFX_H
#define DDRHOOKX_CONFIG_GFX_H

#include "cconfig/cconfig.h"

/**
 * Struct holding configuration values for GFX related items.
 */
struct ddrhookx_config_gfx {
    bool windowed;
};

/**
 * Initialize a cconfig structure with the basic structure and default values
 * of this configuration.
 */
void ddrhookx_config_gfx_init(struct cconfig *config);

/**
 * Read the module specific config struct values from the provided cconfig
 * struct.
 *
 * @param config_gfx Target module specific struct to read configuration
 *                   values to.
 * @param config cconfig struct holding the intermediate data to read from.
 */
void ddrhookx_config_gfx_get(
    struct ddrhookx_config_gfx *config_gfx, struct cconfig *config);

#endif
