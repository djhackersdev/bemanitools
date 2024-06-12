#ifndef PROCMON_CONFIG_H
#define PROCMON_CONFIG_H

#include <stdint.h>

#include "api/core/config.h"

typedef struct procmon_config {
    bool file_enable;
    bool module_enable;
    bool thread_enable;
} procmon_config_t;

void procmon_config_load(
    const bt_core_config_t *config, procmon_config_t *procmon_config);

#endif