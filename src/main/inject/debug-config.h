#ifndef INJECT_DEBUG_CONFIG_H
#define INJECT_DEBUG_CONFIG_H

#include <stdbool.h>

#include "core/property-node.h"

typedef struct debug_config {
    bool property_configs_log;
    bool property_api_trace_log;
} debug_config_t;

void debug_config_init(debug_config_t *config);

void debug_config_load(
    const core_property_node_t *node, debug_config_t *config);

#endif