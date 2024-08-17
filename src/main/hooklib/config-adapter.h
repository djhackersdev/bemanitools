#ifndef HOOKLIB_CONFIG_ADAPTER_H
#define HOOKLIB_CONFIG_ADAPTER_H

#include <windows.h>

#include "cconfig/cconfig.h"

#include "iface-core/config.h"

struct hooklib_config_adapter {
    // this is larger on purpose, in case ppl enter the wrong stuff here
    char override_ip[32];
};

void hooklib_config_adapter_init(struct cconfig *config);

void hooklib_config_adapter_get(
    struct hooklib_config_adapter *config_adapter, struct cconfig *config);

void hooklib_config_adapter_get2(
    const bt_core_config_t *config, struct hooklib_config_adapter *config_adapter);

#endif