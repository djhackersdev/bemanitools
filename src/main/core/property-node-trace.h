#ifndef CORE_PROPERTY_NODE_TRACE_H
#define CORE_PROPERTY_NODE_TRACE_H

#include "main/core/property-node.h"

void core_property_node_trace_target_api_set(
    const core_property_node_api_t *target_api);

void core_property_node_trace_core_api_get(core_property_node_api_t *api);

#endif
