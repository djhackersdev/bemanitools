#define LOG_MODULE "procmon-config"

#include "avs/error.h"

#include "core/log.h"

#include "procmon/config.h"

// clang-format off
PSMAP_BEGIN(procmon_config_psmap)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct procmon_config, file_monitor_enable,
    "procmon/file_monitor_enable", false)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct procmon_config, module_monitor_enable,
    "procmon/module_monitor_enable", false)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct procmon_config, thread_monitor_enable,
    "procmon/thread_monitor_enable", false)
PSMAP_END
// clang-format on

void procmon_config_init(struct procmon_config *config)
{
    log_assert(config);

    memset(config, 0, sizeof(struct procmon_config));
}

void procmon_config_load(
    struct property_node *property_node, struct procmon_config *config)
{
    avs_error error;
    struct property_node *node;

    log_assert(property_node);
    log_assert(config);

    node = property_search(NULL, property_node, "procmon/version");

    if (!node) {
        log_fatal("Missing 'version' node in configuration");
    }

    error = property_node_read(
        node, PROPERTY_TYPE_U32, &config->version, sizeof(config->version));

    if (AVS_IS_ERROR(error)) {
        log_fatal(
            "Reading 'version' node failed: %s", avs_util_error_str(error));
    }

    if (config->version != 1) {
        log_fatal("Unsupported configuration version: %d", config->version);
    }

    error = property_psmap_import(
        NULL, property_node, config, procmon_config_psmap);

    if (AVS_IS_ERROR(error)) {
        log_fatal(
            "Importing config to psmap failed: %s", avs_util_error_str(error));
    }
}

void procmon_config_fini(struct procmon_config *config)
{
    log_assert(config);

    memset(config, 0, sizeof(struct procmon_config));
}