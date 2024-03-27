#include "avs/log.h"
#include "avs/property-node.h"
#include "avs/property.h"
#include "avs/thread.h"

#include "core/log.h"
#include "core/property-node.h"
#include "core/property.h"
#include "core/thread.h"

#include "imports/avs.h"

void avs_core_interop_log_avs_impl_set()
{
    core_log_impl_t impl;
    
    avs_log_impl_get(&impl);
    core_log_impl_set(&impl);
}

void avs_core_interop_property_impl_set()
{
    core_property_impl_t property_impl;
    core_property_node_impl_t property_node_impl;

    avs_property_impl_get(&property_impl);
    core_property_impl_set(&property_impl);

    avs_property_node_impl_get(&property_node_impl);
    core_property_node_impl_set(&property_node_impl);
}

void avs_core_interop_thread_avs_impl_set()
{
    core_thread_impl_t impl;
    
    avs_thread_impl_get(&impl);
    core_thread_impl_set(&impl);
}