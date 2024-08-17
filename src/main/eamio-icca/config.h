#ifndef EAMIO_ICCA_CONFIG_ICC_H
#define EAMIO_ICCA_CONFIG_ICC_H

#include "api/core/config.h"

typedef struct icc_config {
    char port[64];
    int32_t baud;
} icc_config_t;

void eamio_icca_config_icc_get(
    const bt_core_config_t *config, icc_config_t *config_out);

#endif
