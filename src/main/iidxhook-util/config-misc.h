#ifndef IIDXHOOK_CONFIG_MISC_H
#define IIDXHOOK_CONFIG_MISC_H

#include "cconfig/cconfig.h"

struct iidxhook_config_misc {
    bool disable_clock_set;
    bool rteffect_stub;
};

void iidxhook_config_misc_init(struct cconfig* config);

void iidxhook_config_misc_get(struct iidxhook_config_misc* config_misc, 
        struct cconfig* config);

#endif