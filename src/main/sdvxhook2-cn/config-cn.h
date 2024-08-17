#ifndef SDVXHOOK2_CN_CONFIG_H
#define SDVXHOOK2_CN_CONFIG_H

#include <stdbool.h>

#include "api/core/config.h"

typedef struct sdvxhook2_cn_config {
    bool disable_io_emu;
    char unis_path[256];
} sdvxhook2_cn_config_t;

void sdvxhook2_cn_config_get(
    const bt_core_config_t *config, sdvxhook2_cn_config_t *config_out);

#endif