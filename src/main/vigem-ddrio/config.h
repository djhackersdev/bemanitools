#ifndef VIGEM_DDRIO_CONFIG_H
#define VIGEM_DDRIO_CONFIG_H

#include "api/core/config.h"

typedef struct vigem_ddrio_config {
    bool enable_reactive_light;
} vigem_ddrio_config_t;

void vigem_ddrio_config_get(const bt_core_config_t *config, vigem_ddrio_config_t *config_out);

#endif