#define LOG_MODULE "property-trace"

#include <string.h>

#include "core/property.h"

#include "iface-core/log.h"

static core_property_api_t _core_property_trace_target_api;

static core_property_result_t
_core_property_trace_create(size_t size, core_property_t **property)
{
    core_property_result_t result;

    log_misc(">>> create(%d)", size);

    result = _core_property_trace_target_api.v1.create(size, property);

    log_misc(
        "<<< create(%d, %p): %s",
        size,
        *property,
        core_property_result_to_str(result));

    return result;
}

static core_property_result_t
_core_property_trace_file_load(const char *path, core_property_t **property)
{
    core_property_result_t result;

    log_misc(">>> file_load(%s)", path);

    result = _core_property_trace_target_api.v1.file_load(path, property);

    log_misc(
        "<<< file_load(%s, %p): %s",
        path,
        *property,
        core_property_result_to_str(result));

    return result;
}

static core_property_result_t
_core_property_trace_str_load(const char *str, core_property_t **property)
{
    core_property_result_t result;

    log_misc(">>> str_load(%p)", str);

    result = _core_property_trace_target_api.v1.str_load(str, property);

    log_misc(
        "<<< str_load(%p, %p): %s",
        str,
        *property,
        core_property_result_to_str(result));

    return result;
}

static core_property_result_t
_core_property_trace_size(const core_property_t *property, size_t *size)
{
    core_property_result_t result;

    log_misc(">>> size(%p)", property);

    result = _core_property_trace_target_api.v1.size(property, size);

    log_misc(
        "<<< size(%p, %d): %s",
        property,
        *size,
        core_property_result_to_str(result));

    return result;
}

static core_property_result_t _core_property_trace_clone(
    const core_property_t *property, core_property_t **property_cloned)
{
    core_property_result_t result;

    log_misc(">>> clone(%p)", property);

    result =
        _core_property_trace_target_api.v1.clone(property, property_cloned);

    log_misc(
        "<<< clone(%p, %p): %s",
        property,
        *property_cloned,
        core_property_result_to_str(result));

    return result;
}

static void _core_property_trace_log(
    const core_property_t *property, bt_core_log_message_t log_message)
{
    log_misc(">>> log(%p)", property);

    _core_property_trace_target_api.v1.log(property, log_message);

    log_misc("<<< log(%p)", property);
}

static core_property_result_t _core_property_trace_root_node_get(
    const core_property_t *property, core_property_node_t *node)
{
    core_property_result_t result;

    log_misc(">>> root_node_get(%p)", property);

    result = _core_property_trace_target_api.v1.root_node_get(property, node);

    log_misc(
        "<<< root_node_get(%p, %p): %s",
        property,
        node,
        core_property_result_to_str(result));

    return result;
}

static core_property_result_t _core_property_trace_other_node_insert(
    core_property_t *property, const core_property_node_t *node)
{
    core_property_result_t result;

    log_misc(">>> other_node_insert(%p, %p)", property, node);

    result =
        _core_property_trace_target_api.v1.other_node_insert(property, node);

    log_misc(
        "<<< other_node_insert(%p, %p): %s",
        property,
        node,
        core_property_result_to_str(result));

    return result;
}

static void _core_property_trace_free(core_property_t **property)
{
    log_misc(">>> free(%p)", *property);

    _core_property_trace_target_api.v1.free(property);

    log_misc("<<< free(%p)", *property);
}

void core_property_trace_target_api_set(const core_property_api_t *target_api)
{
    log_assert(target_api);

    memcpy(
        &_core_property_trace_target_api,
        target_api,
        sizeof(core_property_api_t));
}

void core_property_trace_core_api_get(core_property_api_t *api)
{
    log_assert(api);

    api->version = 1;

    api->v1.create = _core_property_trace_create;
    api->v1.file_load = _core_property_trace_file_load;
    api->v1.str_load = _core_property_trace_str_load;
    api->v1.size = _core_property_trace_size;
    api->v1.clone = _core_property_trace_clone;
    api->v1.log = _core_property_trace_log;
    api->v1.root_node_get = _core_property_trace_root_node_get;
    api->v1.other_node_insert = _core_property_trace_other_node_insert;
    api->v1.free = _core_property_trace_free;
}