#include <stdlib.h>

#include "core/log.h"
#include "core/thread.h"

#define CORE_THREAD_ASSERT_IMPLEMENTED(func, name) \
    while (0) { \
        if (!func) { \
            log_fatal("Function %s not implemented", STRINGIFY(name)); \
        } \
    }

static core_thread_impl_t _core_thread_impl;

void core_thread_impl_set(const core_thread_impl_t *impl)
{
    log_assert(impl);

    if (_core_thread_impl.create) {
        log_warning("Re-initialize");
    }

    CORE_THREAD_ASSERT_IMPLEMENTED(impl->create, create);
    CORE_THREAD_ASSERT_IMPLEMENTED(impl->join, join);
    CORE_THREAD_ASSERT_IMPLEMENTED(impl->destroy, destroy);

    memcpy(_core_thread_impl, impl, sizeof(core_thread_impl_t));
}

const core_thread_impl_t *core_thread_impl_get()
{
    log_assert(_core_thread_impl.create);

    return &_core_thread_impl;
}

const char *core_thread_result_to_str(core_thread_result_t result)
{
    switch (result) {
        case CORE_THREAD_RESULT_SUCCESS:
            return "Success";
        case CORE_THREAD_RESULT_ERROR_INTERNAL:
            return "Internal";
        default:
            return "Undefined error";
    }
}

void core_thread_fatal_on_error(core_thread_result_t result)
{
    switch (result) {
        case CORE_THREAD_RESULT_SUCCESS:
            return;
        case CORE_THREAD_RESULT_ERROR_INTERNAL:
        default:
            log_fatal("Operation on thread failed: %s", core_thread_result_to_str(result));
    }
}

core_thread_result_t core_thread_create(
    int (*proc)(void *), void *ctx, uint32_t stack_sz, unsigned int priority, core_thread_id_t *thread_id)
{
    log_assert(_core_thread_impl.create);
    log_assert(proc);
    log_assert(stack_sz > 0);
    log_assert(thread_id);

    return _core_thread_impl.create(proc, ctx, stack_sz, priority, thread_id);
}

core_thread_result_t core_thread_join(core_thread_id_t thread_id, int *result)
{
    log_assert(_core_thread_impl.create);

    return _core_thread_impl.join(thread_id, result);
}

core_thread_result_t core_thread_destroy(core_thread_id_t thread_id)
{
    log_assert(_core_thread_impl.create);

    return _core_thread_impl.destroy(thread_id);
}
