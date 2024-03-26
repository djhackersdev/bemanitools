#define LOG_MODULE "core-property"

#include "core/property.h"
#include "core/log.h"

#include "util/defs.h"

#define CORE_PROPERTY_ASSERT_IMPLEMENTED(func, name) \
    while (0) { \
        if (!func) { \
            log_fatal("Function %s not implemented", STRINGIFY(name)); \
        } \
    }

static core_property_impl_t _core_property_impl;

void core_property_impl_set(const core_property_impl_t *impl)
{
    log_assert(impl);

    if (_core_property_impl.create) {
        log_warning("Re-initialize");
    }

    CORE_PROPERTY_ASSERT_IMPLEMENTED(impl->create, create);
    CORE_PROPERTY_ASSERT_IMPLEMENTED(impl->file_load, file_load);
    CORE_PROPERTY_ASSERT_IMPLEMENTED(impl->str_load, str_load);
    CORE_PROPERTY_ASSERT_IMPLEMENTED(impl->size, size);
    CORE_PROPERTY_ASSERT_IMPLEMENTED(impl->clone, clone);
    CORE_PROPERTY_ASSERT_IMPLEMENTED(impl->log, log);
    CORE_PROPERTY_ASSERT_IMPLEMENTED(impl->root_node_get, root_node_get);
    CORE_PROPERTY_ASSERT_IMPLEMENTED(impl->free, free);

    memcpy(&_core_property_impl, impl, sizeof(core_property_impl_t));
}

const core_property_impl_t *core_property_impl_get()
{
    log_assert(_core_property_impl.create);

    return &_core_property_impl;
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
            return "File/path not found";
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
            log_fatal("Operation on property failed: %s", core_property_result_to_str(result));
    }
}

core_property_result_t core_property_create(size_t size, core_property_t **property)
{
    log_assert(_core_property_impl.create);
    log_assert(size > 0);
    log_assert(property);
    
    return _core_property_impl.create(size, property);
}

core_property_result_t core_property_file_load(const char *path, core_property_t **property)
{
    log_assert(_core_property_impl.create);
    log_assert(path);
    log_assert(property);
    
    return _core_property_impl.file_load(path, property);
}

core_property_result_t core_property_str_load(const char *str, core_property_t **property)
{
    log_assert(_core_property_impl.create);
    log_assert(str);
    log_assert(property);
    
    return _core_property_impl.str_load(str, property);
}

core_property_result_t core_property_size(const core_property_t *property, size_t *size)
{
    log_assert(_core_property_impl.create);
    log_assert(property);
    log_assert(size);
    
    return _core_property_impl.size(property, size);
}

core_property_result_t core_property_clone(const core_property_t *property, core_property_t **property_cloned)
{
    log_assert(_core_property_impl.create);
    log_assert(property);
    log_assert(property_cloned);
    
    return _core_property_impl.clone(property, property_cloned);
}

void core_property_log(const core_property_t *property, core_log_message_t log_impl)
{
    log_assert(_core_property_impl.create);
    log_assert(property);
    log_assert(log_impl);
    
    _core_property_impl.log(property, log_impl);
}

core_property_result_t core_property_root_node_get(const core_property_t *property, core_property_node_t **node)
{
    log_assert(_core_property_impl.create);
    log_assert(property);
    log_assert(node);
    
    return _core_property_impl.root_node_get(property, node);
}

void core_property_free(core_property_t *property)
{
    log_assert(_core_property_impl.create);
    log_assert(property);
    
    _core_property_impl.free(property);
}
