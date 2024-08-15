#ifndef IIDXHOOK_CONFIG_SEC_H
#define IIDXHOOK_CONFIG_SEC_H

#include "cconfig/cconfig.h"

#include "iface-core/config.h"

#include "security/mcode.h"

typedef struct iidxhook_config_sec {
    struct security_mcode boot_version;
    uint32_t boot_seeds[3];
    struct security_mcode black_plug_mcode;
} iidxhook_config_sec_t;

void iidxhook_config_sec_init(struct cconfig *config);

void iidxhook_config_sec_get(
    struct iidxhook_config_sec *config_sec, struct cconfig *config);

void iidxhook_util_config_sec_get2(
    const bt_core_config_t *config,
    iidxhook_config_sec_t *config_sec);

#endif