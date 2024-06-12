#ifndef AVS_EXT_PROPERTY_NODE_H
#define AVS_EXT_PROPERTY_NODE_H

#include "imports/avs.h"

#include "main/core/property-node.h"

void avs_ext_property_node_core_api_get(core_property_node_api_t *api);

void avs_ext_property_node_core_api_set();

// Don't use this unless you need to interface directly with avs
struct property_node *
avs_ext_property_node_avs_property_node_get(const core_property_node_t *node);

#endif