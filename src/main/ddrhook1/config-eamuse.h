#ifndef DDRHOOK1_CONFIG_EAMUSE_H
#define DDRHOOK1_CONFIG_EAMUSE_H

#include "api/core/config.h"

#include "security/id.h"

#include "util/net.h"

typedef struct ddrhook1_config_eamuse {
    struct net_addr server;
    struct security_id pcbid;
    struct security_id eamid;
} ddrhook1_config_eamuse_t;

void ddrhook1_config_eamuse_get(
    const bt_core_config_t *config,
    ddrhook1_config_eamuse_t *config_eamuse);

#endif