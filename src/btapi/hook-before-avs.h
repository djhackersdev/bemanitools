#ifndef BTAPI_HOOK_BEFORE_AVS_H
#define BTAPI_HOOK_BEFORE_AVS_H

#include "property.h"

typedef bool (*btapi_hook_before_avs_init_t)(struct property_node *property_node_config);

bool btapi_hook_before_avs_init(struct property_node *property_node_config);

#endif