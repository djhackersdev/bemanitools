#ifndef IIDXHOOK_UTIL_CONFIG_EAMUSE_H
#define IIDXHOOK_UTIL_CONFIG_EAMUSE_H

#include "cconfig/cconfig.h"

#include "security/id.h"

#include "util/net.h"

struct iidxhook_util_config_eamuse {
    char card_type[4];
    struct net_addr server;
    struct security_id pcbid;
    struct security_id eamid;
};

void iidxhook_util_config_eamuse_init(struct cconfig *config);

void iidxhook_util_config_eamuse_get(
    struct iidxhook_util_config_eamuse *config_eamuse, struct cconfig *config);

#endif