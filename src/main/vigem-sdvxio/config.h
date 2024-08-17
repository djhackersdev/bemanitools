#ifndef VIGEM_SDVXIO_CONFIG_H
#define VIGEM_SDVXIO_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

#include "api/core/config.h"

typedef struct vigem_sdvxio_config {
    bool enable_keylight;
    bool relative_analog;
    int32_t pwm_wings;
    int32_t pwm_controller;
    int32_t amp_volume;
} vigem_sdvxio_config_t;

void vigem_sdvxio_config_get(
    const bt_core_config_t *config,
    vigem_sdvxio_config_t *config_out);

#endif