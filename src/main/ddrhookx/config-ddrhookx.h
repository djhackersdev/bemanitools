#ifndef DDRHOOKX_CONFIG_DDRHOOKX_H
#define DDRHOOKX_CONFIG_DDRHOOKX_H

#include <windows.h>

#include "cconfig/cconfig.h"

/**
 * Struct holding configuration values for game-specific items.
 */
struct ddrhookx_config_ddrhookx {
    bool use_com4_emu;
    bool standard_def;
    char usbmem_path[MAX_PATH];
};

/**
 * Initialize a cconfig structure with the basic structure and default values
 * of this configuration.
 */
void ddrhookx_config_ddrhookx_init(struct cconfig *config);

/**
 * Read the module specific config struct values from the provided cconfig
 * struct.
 *
 * @param config_ddrhookx Target module specific struct to read configuration
 *                   values to.
 * @param config cconfig struct holding the intermediate data to read from.
 */
void ddrhookx_config_ddrhookx_get(
    struct ddrhookx_config_ddrhookx *config_ddrhookx, struct cconfig *config);

#endif
