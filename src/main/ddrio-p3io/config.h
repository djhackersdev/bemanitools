#ifndef DDRIO_P3IO_CONFIG_H
#define DDRIO_P3IO_CONFIG_H

#include "api/core/config.h"

typedef struct ddrio_p3io_config {
    char extio_port[12];
} ddrio_p3io_config_t;

void ddrio_p3io_config_get(const bt_core_config_t *config, ddrio_p3io_config_t *config_ddrio_p3io);

#endif