#define LOG_MODULE "core-property"

#include <string.h>

#include "core/property-trace.h"
#include "core/property.h"

#include "iface-core/log.h"

#include "util/defs.h"

#define CORE_PROPERTY_ASSERT_IMPLEMENTED(func, name)               \
    if (!func) {                                                   \
        log_fatal("Function %s not implemented", STRINGIFY(name)); \
    }

static bool _core_property_trace_log_enable;
static core_property_api_t _core_property_api;

static bool _core_property_api_is_valid()
{
    return _core_property_api.version > 0;
}

void core_property_trace_log_enable(bool enable)
{
    _core_property_trace_log_enable = enable;
}

void core_property_api_set(const core_property_api_t *api)
{
    core_property_api_t trace_api;

    log_assert(api);

    if (_core_property_api_is_valid()) {
        log_warning("Re-initialize");
    }

    if (api->version == 1) {
        CORE_PROPERTY_ASSERT_IMPLEMENTED(api->v1.create, create);
        CORE_PROPERTY_ASSERT_IMPLEMENTED(api->v1.file_load, file_load);
        CORE_PROPERTY_ASSERT_IMPLEMENTED(api->v1.str_load, str_load);
        CORE_PROPERTY_ASSERT_IMPLEMENTED(api->v1.size, size);
        CORE_PROPERTY_ASSERT_IMPLEMENTED(api->v1.clone, clone);
        CORE_PROPERTY_ASSERT_IMPLEMENTED(api->v1.root_node_get, root_node_get);
        CORE_PROPERTY_ASSERT_IMPLEMENTED(
            api->v1.other_node_insert, other_node_insert);
        CORE_PROPERTY_ASSERT_IMPLEMENTED(api->v1.free, free);
    } else {
        log_fatal("Unsupported API version: %d", api->version);
    }

    if (_core_property_trace_log_enable) {
        log_info("API trace log enabled");

        core_property_trace_target_api_set(api);
        core_property_trace_core_api_get(&trace_api);

        memcpy(&_core_property_api, &trace_api, sizeof(core_property_api_t));
    } else {
        memcpy(&_core_property_api, api, sizeof(core_property_api_t));
    }
}

void core_property_api_get(core_property_api_t *api)
{
    log_assert(api);
    log_assert(_core_property_api_is_valid());

    memcpy(api, &_core_property_api, sizeof(core_property_api_t));
}

const char *core_property_result_to_str(core_property_result_t result)
{
    switch (result) {
        case CORE_PROPERTY_RESULT_SUCCESS:
            return "Success";
        case CORE_PROPERTY_RESULT_ERROR_INTERNAL:
            return "Internal";
        case CORE_PROPERTY_RESULT_ERROR_ALLOC:
            return "Allocating memory";
        case CORE_PROPERTY_RESULT_NOT_FOUND:
            return "File or path not found";
        case CORE_PROPERTY_RESULT_ERROR_PERMISSIONS:
            return "Permissions";
        case CORE_PROPERTY_RESULT_ERROR_READ:
            return "Read error";
        default:
            return "Undefined error";
    }
}

void core_property_fatal_on_error(core_property_result_t result)
{
    switch (result) {
        case CORE_PROPERTY_RESULT_SUCCESS:
            return;
        case CORE_PROPERTY_RESULT_ERROR_INTERNAL:
        case CORE_PROPERTY_RESULT_ERROR_ALLOC:
        case CORE_PROPERTY_RESULT_NOT_FOUND:
        case CORE_PROPERTY_RESULT_ERROR_PERMISSIONS:
        case CORE_PROPERTY_RESULT_ERROR_READ:
        default:
            log_fatal(
                "Operation on property failed: %s",
                core_property_result_to_str(result));
    }
}

core_property_result_t
core_property_create(size_t size, core_property_t **property)
{
    log_assert(_core_property_api_is_valid());
    log_assert(size > 0);
    log_assert(property);

    return _core_property_api.v1.create(size, property);
}

core_property_result_t
core_property_file_load(const char *path, core_property_t **property)
{
    log_assert(_core_property_api_is_valid());
    log_assert(path);
    log_assert(property);

    return _core_property_api.v1.file_load(path, property);
}

core_property_result_t
core_property_str_load(const char *str, core_property_t **property)
{
    log_assert(_core_property_api_is_valid());
    log_assert(str);
    log_assert(property);

    return _core_property_api.v1.str_load(str, property);
}

core_property_result_t
core_property_size(const core_property_t *property, size_t *size)
{
    log_assert(_core_property_api_is_valid());
    log_assert(property);
    log_assert(size);

    return _core_property_api.v1.size(property, size);
}

core_property_result_t core_property_clone(
    const core_property_t *property, core_property_t **property_cloned)
{
    log_assert(_core_property_api_is_valid());
    log_assert(property);
    log_assert(property_cloned);

    return _core_property_api.v1.clone(property, property_cloned);
}

core_property_result_t core_property_root_node_get(
    const core_property_t *property, core_property_node_t *node)
{
    log_assert(_core_property_api_is_valid());
    log_assert(property);
    log_assert(node);

    return _core_property_api.v1.root_node_get(property, node);
}

core_property_result_t core_property_other_node_insert(
    core_property_t *property, const core_property_node_t *node)
{
    log_assert(_core_property_api_is_valid());
    log_assert(property);
    log_assert(node);

    return _core_property_api.v1.other_node_insert(property, node);
}

void core_property_free(core_property_t **property)
{
    log_assert(_core_property_api_is_valid());
    log_assert(property);

    _core_property_api.v1.free(property);
}
