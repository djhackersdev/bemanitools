#define LOG_MODULE "inject-debugger-config"

#include "core/property-node-ext.h"

#include "iface-core/log.h"

#include "inject/debugger-config.h"

#include "util/str.h"

static debugger_attach_type_t _debugger_config_str_to_attachtype(const char *str)
{
    log_assert(str);

    if (str_eq(str, "none")) {
        return DEBUGGER_ATTACH_TYPE_NONE;
    } else if (str_eq(str, "inject")) {
        return DEBUGGER_ATTACH_TYPE_INJECT;
    } else if (str_eq(str, "external")) {
        return DEBUGGER_ATTACH_TYPE_EXTERNAL;
    } else {
        log_fatal("Invalid debugger attach type for debugger config: %s", str);
    }
}

void debugger_config_init(debugger_config_t *config)
{
    log_assert(config);

    memset(config, 0, sizeof(debugger_config_t));
}

void debugger_config_load(
    const core_property_node_t *node, debugger_config_t *config)
{
    core_property_node_result_t result;
    char buffer[16];

    log_assert(node);
    log_assert(config);

    result = core_property_node_ext_str_read(node, "app/path", config->app.path, sizeof(config->app.path));
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_str_read_or_default(
        node, "app/args", config->app.args, sizeof(config->app.args), "");
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_str_read(node, "attach_type", buffer, sizeof(buffer));
    core_property_node_fatal_on_error(result);
    config->attach_type = _debugger_config_str_to_attachtype(buffer);
}