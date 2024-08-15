#define LOG_MODULE "bt-hook-inject-config"

#include "core/property-ext.h"
#include "core/property-node.h"
#include "core/property.h"

#include "core/property-mxml-internal.h"
#include "core/property-node-ext.h"

#include "iface-core/log.h"

#include "sdk-hook/hooks-config.h"
#include "sdk-hook/inject-config.h"
#include "sdk-hook/logger-config.h"

void bt_hook_inject_config_init(struct bt_hook_inject_config *config)
{
    log_assert(config);

    config->version = 1;

    bt_hook_hooks_config_init(&config->hooks);
    bt_hook_logger_config_init(&config->logger);
}

void bt_hook_inject_config_file_load(const char *path, bt_hook_inject_config_t *config)
{
    core_property_result_t result_prop;
    core_property_node_result_t result;
    core_property_t *property;
    core_property_node_t root_node;
    core_property_node_t child_node;

    log_info("Loading configuration file: %s", path);

    result_prop = core_property_file_load(path, &property);
    core_property_fatal_on_error(result_prop);

    result = core_property_root_node_get(property, &root_node);
    core_property_node_fatal_on_error(result);

    result = core_property_node_search(&root_node, "hooks", &child_node);
    core_property_node_fatal_on_error(result);
    bt_hook_hooks_config_load(&child_node, &config->hooks);

    result = core_property_node_search(&root_node, "logger", &child_node);
    core_property_node_fatal_on_error(result);
    bt_hook_logger_config_load(&child_node, &config->logger);

    core_property_free(&property);

    log_misc("Loading done");
}

void bt_hook_inject_config_fini(bt_hook_inject_config_t *config)
{
    log_assert(config);

    // Other configs don't have a fini
    bt_hook_hooks_config_fini(&config->hooks);
}