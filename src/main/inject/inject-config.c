#define LOG_MODULE "inject-config"

#include "core/property-ext.h"
#include "core/property-node.h"
#include "core/property.h"

#include "core/property-mxml-internal.h"
#include "core/property-node-ext.h"

#include "iface-core/log.h"

#include "inject/debug-config.h"
#include "inject/debugger-config.h"
#include "inject/hooks-config.h"
#include "inject/inject-config.h"
#include "inject/logger-config.h"

void inject_config_init(struct inject_config *config)
{
    log_assert(config);

    config->version = 1;

    debug_config_init(&config->debug);
    debugger_config_init(&config->debugger);
    hooks_config_init(&config->hooks);
    logger_config_init(&config->logger);
}

void inject_config_file_load(const char *path, inject_config_t *config)
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

    result = core_property_node_search(&root_node, "debug", &child_node);
    core_property_node_fatal_on_error(result);
    debug_config_load(&child_node, &config->debug);

    result = core_property_node_search(&root_node, "debugger", &child_node);
    core_property_node_fatal_on_error(result);
    debugger_config_load(&child_node, &config->debugger);

    result = core_property_node_search(&root_node, "hooks", &child_node);
    core_property_node_fatal_on_error(result);
    hooks_config_load(&child_node, &config->hooks);

    result = core_property_node_search(&root_node, "logger", &child_node);
    core_property_node_fatal_on_error(result);
    logger_config_load(&child_node, &config->logger);

    if (config->debug.property_configs_log) {
        core_property_ext_log(property, log_misc_func);
    }

    core_property_free(&property);

    log_misc("Loading done");
}

void inject_config_fini(inject_config_t *config)
{
    log_assert(config);

    // Other configs don't have a fini
    hooks_config_fini(&config->hooks);
}