#ifndef VIGEM_SDVXIO_CONFIG_H
#define VIGEM_SDVXIO_CONFIG_H

#include <windows.h>

#include "cconfig/cconfig.h"

struct vigem_sdvxio_config {
    bool enable_keylight;
    int32_t pwm_wings;
    int32_t pwm_controller;
    int32_t amp_volume;
};

void get_vigem_sdvxio_config(struct vigem_sdvxio_config *config_out);

#endif