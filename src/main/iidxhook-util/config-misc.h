#ifndef IIDXHOOK_CONFIG_MISC_H
#define IIDXHOOK_CONFIG_MISC_H

#include <windows.h>

#include "cconfig/cconfig.h"

#include "iface-core/config.h"

typedef struct iidxhook_config_misc {
    bool disable_clock_set;
    bool rteffect_stub;
    char settings_path[MAX_PATH];
} iidxhook_config_misc_t;

void iidxhook_config_misc_init(struct cconfig *config);

void iidxhook_config_misc_get(
    struct iidxhook_config_misc *config_misc, struct cconfig *config);

void iidxhook_util_config_misc_get2(
    const bt_core_config_t *config,
    iidxhook_config_misc_t *config_misc);

#endif