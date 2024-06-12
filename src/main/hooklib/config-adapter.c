#include "cconfig/cconfig-util.h"

#include "iface-core/log.h"

#include "hooklib/config-adapter.h"

#define HOOKLIB_CONFIG_ADAPTER_OVERRIDE_IP_KEY "adapter.override_ip"

#define HOOKLIB_CONFIG_ADAPTER_DEFAULT_OVERRIDE_IP_VALUE ""

void hooklib_config_adapter_init(struct cconfig *config)
{
    cconfig_util_set_str(
        config,
        HOOKLIB_CONFIG_ADAPTER_OVERRIDE_IP_KEY,
        HOOKLIB_CONFIG_ADAPTER_DEFAULT_OVERRIDE_IP_VALUE,
        "IP of adapter to force override with");
}

void hooklib_config_adapter_get(
    struct hooklib_config_adapter *config_adapter, struct cconfig *config)
{
    if (!cconfig_util_get_str(
            config,
            HOOKLIB_CONFIG_ADAPTER_OVERRIDE_IP_KEY,
            config_adapter->override_ip,
            sizeof(config_adapter->override_ip),
            HOOKLIB_CONFIG_ADAPTER_DEFAULT_OVERRIDE_IP_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%s'",
            HOOKLIB_CONFIG_ADAPTER_OVERRIDE_IP_KEY,
            HOOKLIB_CONFIG_ADAPTER_DEFAULT_OVERRIDE_IP_VALUE);
    }
}
