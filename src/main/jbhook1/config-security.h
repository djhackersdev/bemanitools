#ifndef JBHOOK1_CONFIG_SECURITY_H
#define JBHOOK1_CONFIG_SECURITY_H

#include "api/core/config.h"

#include "security/mcode.h"

/**
 * Struct holding configuration values for security related items.
 */
typedef struct jbhook1_config_security {
    struct security_mcode mcode;
} jbhook1_config_security_t;

void jbhook1_config_security_get(
    const bt_core_config_t *config,
    jbhook1_config_security_t *config_out);

#endif