#ifndef POPNHOOK1_CONFIG_SEC_H
#define POPNHOOK1_CONFIG_SEC_H

#include "api/core/config.h"

#include "security/mcode.h"

typedef struct popnhook1_config_sec {
    security_mcode_t black_plug_mcode;
} popnhook1_config_sec_t;

void popnhook1_config_sec_get(
    const bt_core_config_t *config,
    popnhook1_config_sec_t *out_config);
    
#endif