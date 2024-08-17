#ifndef POPNHOOK1_CONFIG_EAMUSE_H
#define POPNHOOK1_CONFIG_EAMUSE_H

#include "api/core/config.h"

#include "security/id.h"

#include "util/net.h"

typedef struct popnhook1_config_eamuse {
    struct net_addr server;
    struct security_id pcbid;
    struct security_id eamid;
} popnhook1_config_eamuse_t;

void popnhook1_config_eamuse_get(
    const bt_core_config_t *config,
    popnhook1_config_eamuse_t *config_out);

#endif