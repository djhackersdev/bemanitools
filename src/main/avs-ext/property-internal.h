#ifndef AVS_EXT_PROPERTY_INTERNAL_H
#define AVS_EXT_PROPERTY_INTERNAL_H

#include "core/property-node.h"
#include "core/property.h"

#include "imports/avs.h"

typedef struct avs_ext_property_internal_property {
    struct property *property;
} avs_ext_property_internal_property_t;

_Static_assert(
    sizeof(avs_ext_property_internal_property_t) <= sizeof(core_property_t),
    "Not enough space for stack allocations");

typedef struct avs_ext_property_internal_node {
    avs_ext_property_internal_property_t *property;
    struct property_node *node;
} avs_ext_property_internal_node_t;

_Static_assert(
    sizeof(avs_ext_property_internal_node_t) <= sizeof(core_property_node_t),
    "Not enough space for stack allocations");

#endif