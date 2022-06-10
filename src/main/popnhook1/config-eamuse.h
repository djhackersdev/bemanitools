#ifndef POPNHOOK1_CONFIG_EAMUSE_H
#define POPNHOOK1_CONFIG_EAMUSE_H

#include "cconfig/cconfig.h"

#include "security/id.h"

#include "util/net.h"

struct popnhook1_config_eamuse {
    struct net_addr server;
    struct security_id pcbid;
    struct security_id eamid;
};

void popnhook1_config_eamuse_init(struct cconfig *config);

void popnhook1_config_eamuse_get(
    struct popnhook1_config_eamuse *config_eamuse, struct cconfig *config);

#endif