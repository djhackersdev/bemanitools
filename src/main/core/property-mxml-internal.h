#ifndef CORE_PROPERTY_MXML_INTERNAL_H
#define CORE_PROPERTY_MXML_INTERNAL_H

#include <mxml/mxml.h>

#include "core/property.h"
#include "core/property-node.h"

// As there is no need to have a fixed size allocated for properties with
// everything dynamically growing on the heap when necessary, just
// use this as a dummy value to be compatible with the property interface
#define CORE_PROPERTY_MXML_INTERNAL_FIXED_SIZE_DUMMY 573

typedef struct core_property_mxml_internal_property {
    mxml_node_t *document;
} core_property_mxml_internal_property_t;

_Static_assert(
    sizeof(core_property_mxml_internal_property_t) <= sizeof(core_property_t),
    "Not enough space for stack allocations");

typedef struct core_property_mxml_internal_property_node {
    core_property_mxml_internal_property_t *property;
    mxml_node_t *node;
    // Used when node is being iterated to remember the root of the iteration
    mxml_node_t *node_root_iter;
} core_property_mxml_internal_property_node_t;

_Static_assert(
    sizeof(core_property_mxml_internal_property_node_t) <= sizeof(core_property_node_t),
    "Not enough space for stack allocations");

#endif