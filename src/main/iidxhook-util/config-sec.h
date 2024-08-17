#ifndef IIDXHOOK_CONFIG_SEC_H
#define IIDXHOOK_CONFIG_SEC_H

#include "iface-core/config.h"

#include "security/mcode.h"

typedef struct iidxhook_config_sec {
    struct security_mcode boot_version;
    uint32_t boot_seeds[3];
    struct security_mcode black_plug_mcode;
} iidxhook_config_sec_t;

void iidxhook_util_config_sec_get(
    const bt_core_config_t *config,
    iidxhook_config_sec_t *config_sec);

#endif