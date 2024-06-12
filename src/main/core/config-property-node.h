#ifndef CORE_CONFIG_PROPERTY_NODE_H
#define CORE_CONFIG_PROPERTY_NODE_H

#include "iface-core/config.h"

#include "core/property-node.h"

void core_config_property_node_core_api_set();

void core_config_property_node_init(
    const core_property_node_t *node, bt_core_config_t **config);

void core_config_property_node_free(bt_core_config_t **config);

#endif