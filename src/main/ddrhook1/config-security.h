#ifndef DDRHOOK1_CONFIG_SECURITY_H
#define DDRHOOK1_CONFIG_SECURITY_H

#include "api/core/config.h"

#include "security/mcode.h"

typedef struct ddrhook1_config_security {
    struct security_mcode mcode;
} ddrhook1_config_security_t;

void ddrhook1_config_security_get(
    const bt_core_config_t *config,
    ddrhook1_config_security_t *config_security);

#endif