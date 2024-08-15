#define LOG_MODULE "core-property-ext"

#include "iface-core/log.h"

#include "main/core/property-node-ext.h"
#include "main/core/property.h"

core_property_result_t core_property_ext_many_merge(
    const core_property_t **properties,
    size_t count,
    core_property_t **out_property)
{
    core_property_t *merged_property;
    core_property_t *tmp;
    core_property_result_t result;
    core_property_node_result_t result2;
    int i;

    log_assert(properties);
    log_assert(count > 0);
    log_assert(out_property);

    result = core_property_clone(properties[0], &merged_property);

    if (CORE_PROPERTY_RESULT_IS_ERROR(result)) {
        return result;
    }

    if (count == 1) {
        *out_property = merged_property;
        return CORE_PROPERTY_RESULT_SUCCESS;
    }

    core_property_log(merged_property, log_misc_func);

    for (i = 0; i < count; i++) {
        core_property_log(properties[i], log_misc_func);
    }

    for (i = 1; i < count; i++) {
        result2 = core_property_node_ext_merge_do(
            merged_property, properties[i], &tmp);

        core_property_log(tmp, log_misc_func);

        if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result2)) {
            core_property_free(&merged_property);
            return CORE_PROPERTY_RESULT_ERROR_INTERNAL;
        }

        core_property_free(&merged_property);
        merged_property = tmp;
    }

    *out_property = merged_property;

    return CORE_PROPERTY_RESULT_SUCCESS;
}

core_property_result_t core_property_ext_merge(
    const core_property_t *property_1,
    const core_property_t *property_2,
    core_property_t **out_property)
{
    const core_property_t *properties[2];

    log_assert(property_1);
    log_assert(property_2);
    log_assert(out_property);

    properties[0] = property_1;
    properties[1] = property_2;

    return core_property_ext_many_merge(properties, 2, out_property);
}