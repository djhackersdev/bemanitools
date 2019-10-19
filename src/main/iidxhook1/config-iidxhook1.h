#ifndef IIDXHOOK_CONFIG_IIDXHOOK1_H
#define IIDXHOOK_CONFIG_IIDXHOOK1_H

#include "cconfig/cconfig.h"

struct iidxhook_config_iidxhook1 {
    bool happy_sky_ms_bg_fix;
};

void iidxhook_config_iidxhook1_init(struct cconfig *config);

void iidxhook_config_iidxhook1_get(
    struct iidxhook_config_iidxhook1 *config_iidxhook1, struct cconfig *config);

#endif