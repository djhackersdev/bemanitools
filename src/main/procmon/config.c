#define LOG_MODULE "procmon-config"

#include "core/log.h"

#include "procmon/config.h"

static void _procmon_config_init_default(procmon_config_t *config)
{
    config->file_enable = false;
    config->module_enable = false;
    config->thread_enable = false;
}

void procmon_config_load(
        const bt_core_config_t *config, procmon_config_t *config)
{
    uint32_t version;

    log_assert(config);
    log_assert(config);

    _procmon_config_init_default(config);

    version = 0;

    bt_core_config_u32_get(config, "/version", &config->version);
    
    if (version != 1) {
        log_fatal("Unsupported configuration version: %d", version);
    }
    
    bt_core_config_bool_get(config, "/file/enable", &config->file_enable);
    bt_core_config_bool_get(config, "/module/enable", &config->module_enable);
    bt_core_config_bool_get(config, "/thread/enable", &config->thread_enable);
}