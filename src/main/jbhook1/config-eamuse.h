#ifndef JBHOOK1_CONFIG_EAMUSE_H
#define JBHOOK1_CONFIG_EAMUSE_H

#include "api/core/config.h"

#include "security/id.h"

#include "util/net.h"

/**
 * Struct holding configuration values for eamuse related items.
 */
typedef struct jbhook1_config_eamuse {
    struct net_addr server;
    struct security_id pcbid;
    struct security_id eamid;
} jbhook1_config_eamuse_t;

void jbhook1_config_eamuse_get(
    const bt_core_config_t *config,
    jbhook1_config_eamuse_t *config_out);

#endif