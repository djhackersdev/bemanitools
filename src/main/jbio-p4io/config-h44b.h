#ifndef JBIO_CONFIG_H44B_H
#define JBIO_CONFIG_H44B_H

#include "api/core/config.h"

typedef struct h44b_config {
    char port[64];
    int32_t baud;
} h44b_config_t;

void jbio_config_h44b_get(
    const bt_core_config_t *config,
    h44b_config_t *config_out);

#endif
