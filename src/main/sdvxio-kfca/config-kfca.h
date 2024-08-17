#ifndef SDVXIO_KFCA_CONFIG_KFCA_H
#define SDVXIO_KFCA_CONFIG_KFCA_H

#include <stdint.h>

#include "api/core/config.h"

typedef struct sdvxio_kfca_config_kfca {
    char port[64];
    int32_t baud;
    int32_t override_main_volume;
    int32_t override_headphone_volume;
    int32_t override_sub_volume;
} sdvxio_kfca_config_kfca_t;

void sdvxio_kfca_config_kfca_get(
    const bt_core_config_t *config,
    sdvxio_kfca_config_kfca_t *config_out);
    
#endif