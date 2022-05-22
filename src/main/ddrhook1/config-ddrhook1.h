#ifndef DDRHOOK1_CONFIG_DDRHOOK1_H
#define DDRHOOK1_CONFIG_DDRHOOK1_H

#include <windows.h>

#include "cconfig/cconfig.h"

/**
 * Struct holding configuration values for game-specific items.
 */
struct ddrhook1_config_ddrhook1 {
    bool use_com4_emu;
    bool standard_def;
    bool use_15khz;
    bool usbmem_enabled;
    char usbmem_path_p1[MAX_PATH];
    char usbmem_path_p2[MAX_PATH];
};

/**
 * Initialize a cconfig structure with the basic structure and default values
 * of this configuration.
 */
void ddrhook1_config_ddrhook1_init(struct cconfig *config);

/**
 * Read the module specific config struct values from the provided cconfig
 * struct.
 *
 * @param config_ddrhook1 Target module specific struct to read configuration
 *                   values to.
 * @param config cconfig struct holding the intermediate data to read from.
 */
void ddrhook1_config_ddrhook1_get(
    struct ddrhook1_config_ddrhook1 *config_ddrhook1, struct cconfig *config);

#endif
