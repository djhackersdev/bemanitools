#ifndef CORE_CONFIG_PROPERTY_NODE_H
#define CORE_CONFIG_PROPERTY_NODE_H

#include "core/config.h"
#include "core/property-node.h"

typedef struct core_config_property_node core_config_property_node_t;

const core_config_impl_t *core_config_property_node_impl_get();

void core_config_property_node_init(core_config_property_node_t *node, core_config_property_node_t **config);

void core_config_property_node_free(core_config_property_node_t **config);

#endif