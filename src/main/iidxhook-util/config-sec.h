#ifndef IIDXHOOK_CONFIG_SEC_H
#define IIDXHOOK_CONFIG_SEC_H

#include "cconfig/cconfig.h"

#include "security/mcode.h"

struct iidxhook_config_sec {
    struct security_mcode boot_version;
    uint32_t boot_seeds[3];
    struct security_mcode black_plug_mcode;
};

void iidxhook_config_sec_init(struct cconfig *config);

void iidxhook_config_sec_get(
    struct iidxhook_config_sec *config_sec, struct cconfig *config);

#endif