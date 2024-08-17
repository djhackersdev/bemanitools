#ifndef HOOKLIB_CONFIG_ADAPTER_H
#define HOOKLIB_CONFIG_ADAPTER_H

#include "iface-core/config.h"

struct hooklib_config_adapter {
    // this is larger on purpose, in case ppl enter the wrong stuff here
    char override_ip[32];
};

void hooklib_config_adapter_get(
    const bt_core_config_t *config, struct hooklib_config_adapter *config_adapter);

#endif