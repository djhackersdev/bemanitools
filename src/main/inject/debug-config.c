#define LOG_MODULE "inject-debug-config"

#include "core/property-node-ext.h"

#include "iface-core/log.h"

#include "inject/debug-config.h"

void debug_config_init(debug_config_t *config)
{
    log_assert(config);

    config->property_configs_log = false;
}

void debug_config_load(
    const core_property_node_t *node, debug_config_t *config)
{
    core_property_node_result_t result;

    log_assert(node);
    log_assert(config);

    result = core_property_node_ext_bool_read(node, "property_configs_log", &config->property_configs_log);
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_bool_read(node, "property_api_trace_log", &config->property_configs_log);
    core_property_node_fatal_on_error(result);
}