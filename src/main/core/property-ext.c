#define LOG_MODULE "core-property-ext"

#include "core/log.h"
#include "core/property.h"

core_property_result_t core_property_ext_extract(
        const core_property_node_t *node, core_property_t **out_property)
{
    core_property_result_t result;
    core_property_node_result_t result2;
    core_property_t *tmp;

    // Hack: Is it even possible to get the size of a (sub-) node without
    // the property? 256kb should be fine for now, even for larger
    // configurations. Obviously, this scales horribly and wastes a lot of
    // memory for most smaller sub-nodes
    result = core_property_create(1024 * 256, &tmp);

    if (CORE_PROPERTY_RESULT_IS_ERROR(result)) {
        return result;
    }

    result2 = core_property_node_copy(tmp, node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result2)) {
        core_property_free(tmp);
        return CORE_PROPERTY_RESULT_ERROR_INTERNAL; 
    }

    *out_property = tmp;

    return CORE_PROPERTY_RESULT_SUCCESS;
}

core_property_result_t core_property_ext_many_merge(core_property_t **properties, size_t count, core_property_t **out_property)
{
    core_property_t *merged_property;
    core_property_t *tmp;
    core_property_result_t result;
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

    for (i = 1; i < count; i++) {
        result = core_property_ext_merge(merged_property, properties[i], &tmp);

        if (CORE_PROPERTY_RESULT_IS_ERROR(result)) {
            core_property_free(merged_property);
            return result;
        }

        core_property_free(merged_property);
        merged_property = tmp;
    }

    *out_property = merged_property;

    return CORE_PROPERTY_RESULT_SUCCESS;
}

core_property_result_t core_property_ext_merge(core_property_t *parent, core_property_t *source, core_property_t **out_property)
{

}