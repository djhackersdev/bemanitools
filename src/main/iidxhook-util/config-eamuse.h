#ifndef IIDXHOOK_UTIL_CONFIG_EAMUSE_H
#define IIDXHOOK_UTIL_CONFIG_EAMUSE_H

#include "cconfig/cconfig.h"

#include "iface-core/config.h"

#include "security/id.h"

#include "util/net.h"

typedef struct iidxhook_util_config_eamuse {
    char card_type[4];
    struct net_addr server;
    struct security_id pcbid;
    struct security_id eamid;
} iidxhook_util_config_eamuse_t;

void iidxhook_util_config_eamuse_init(struct cconfig *config);

void iidxhook_util_config_eamuse_get(
    iidxhook_util_config_eamuse_t *config_eamuse, struct cconfig *config);

void iidxhook_util_config_eamuse_get2(
    const bt_core_config_t *config,
    iidxhook_util_config_eamuse_t *config_eamuse);

#endif