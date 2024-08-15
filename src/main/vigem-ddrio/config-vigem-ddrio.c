#include "cconfig/cconfig-main.h"
#include "cconfig/cconfig-util.h"

#include "iface-core/log.h"

#include "vigem-ddrio/config-vigem-ddrio.h"

#define VIGEM_DDRIO_CONFIG_ENABLE_REACTIVE_LIGHT_KEY \
    "ddrio.enable_reactive_light"

#define VIGEM_DDRIO_CONFIG_DEFAULT_ENABLE_REACTIVE_LIGHT_VALUE true

static void vigem_ddrio_config_init(struct cconfig *config)
{
    cconfig_util_set_bool(
        config,
        VIGEM_DDRIO_CONFIG_ENABLE_REACTIVE_LIGHT_KEY,
        VIGEM_DDRIO_CONFIG_DEFAULT_ENABLE_REACTIVE_LIGHT_VALUE,
        "Enable reactive lights based on input.");
}

static void vigem_ddrio_config_get(
    struct vigem_ddrio_config *vigem_config, struct cconfig *config)
{
    if (!cconfig_util_get_bool(
            config,
            VIGEM_DDRIO_CONFIG_ENABLE_REACTIVE_LIGHT_KEY,
            &vigem_config->enable_reactive_light,
            VIGEM_DDRIO_CONFIG_DEFAULT_ENABLE_REACTIVE_LIGHT_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            VIGEM_DDRIO_CONFIG_ENABLE_REACTIVE_LIGHT_KEY,
            VIGEM_DDRIO_CONFIG_DEFAULT_ENABLE_REACTIVE_LIGHT_VALUE);
    }
}

bool get_vigem_ddrio_config(struct vigem_ddrio_config *config_out)
{
    struct cconfig *config;

    config = cconfig_init();

    vigem_ddrio_config_init(config);

    if (!cconfig_main_config_init(
            config,
            "--config",
            "vigem-ddrio.conf",
            "--help",
            "-h",
            "vigem-ddrio",
            CCONFIG_CMD_USAGE_OUT_STDOUT)) {
        cconfig_finit(config);
        return false;
    }

    vigem_ddrio_config_get(config_out, config);

    cconfig_finit(config);

    return true;
}
