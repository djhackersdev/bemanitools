#ifndef BIO2DRV_CONFIG_BIO2_H
#define BIO2DRV_CONFIG_BIO2_H

#include "api/core/config.h"

typedef struct bio2drv_config {
    bool autodetect;
    char port[64];
    int32_t baud;
} bio2drv_config_t;

void bio2drv_config_bio2_get(
    const bt_core_config_t *config, bio2drv_config_t *config_out);

#endif
