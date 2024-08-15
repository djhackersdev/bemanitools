#ifndef CORE_PROPERTY_EXT_H
#define CORE_PROPERTY_EXT_H

#include "iface-core/log.h"

#include "main/core/property.h"

void core_property_ext_log(
    const core_property_t *property, bt_core_log_message_t log_message);

core_property_result_t core_property_ext_many_merge(
    core_property_t **properties, size_t count, core_property_t **out_property);

core_property_result_t core_property_ext_merge(
    const core_property_t *property_1,
    const core_property_t *property_2,
    core_property_t **out_property);

#endif
