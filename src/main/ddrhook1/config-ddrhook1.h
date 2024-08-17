#ifndef DDRHOOK1_CONFIG_DDRHOOK1_H
#define DDRHOOK1_CONFIG_DDRHOOK1_H

#include <windows.h>

#include <stdbool.h>

#include "api/core/config.h"

typedef struct ddrhook1_config_ddrhook1 {
    bool use_com4_emu;
    bool standard_def;
    bool use_15khz;
    bool usbmem_enabled;
    char usbmem_path_p1[MAX_PATH];
    char usbmem_path_p2[MAX_PATH];
} ddrhook1_config_ddrhook1_t;

void ddrhook1_config_ddrhook1_get(
    const bt_core_config_t *config,
    ddrhook1_config_ddrhook1_t *config_ddrhook1);

#endif
