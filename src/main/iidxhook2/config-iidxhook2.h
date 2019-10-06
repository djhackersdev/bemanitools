#ifndef IIDXHOOK_CONFIG_IIDXHOOK2_H
#define IIDXHOOK_CONFIG_IIDXHOOK2_H

#include "cconfig/cconfig.h"

struct iidxhook_config_iidxhook2 {
    bool distorted_ms_bg_fix;
};

void iidxhook_config_iidxhook2_init(struct cconfig* config);

void iidxhook_config_iidxhook2_get(
        struct iidxhook_config_iidxhook2* config_iidxhook2, 
        struct cconfig* config);

#endif