#ifndef AVS_PROPERTY_INTERNAL_H
#define AVS_PROPERTY_INTERNAL_H

#include "imports/avs.h"

struct avs_property_internal_property {
    struct property *property;
};

typedef struct avs_property_internal_property avs_property_internal_property_t;

struct avs_property_internal_node {
    avs_property_internal_property_t *property;
    struct property_node *node;
};

typedef struct avs_property_internal_node avs_property_internal_node_t;

#endif