#include <stdlib.h>

#include "btsdk/core/log.h"
#include "btsdk/core/thread.h"

static bt_core_thread_impl_t _bt_core_thread_impl;

void bt_core_thread_impl_set(const bt_core_thread_impl_t *impl)
{
    log_assert(impl);

    if (_bt_core_thread_impl.create) {
        log_warning("Re-initialize");
    }

    memcpy(_bt_core_thread_impl, impl, sizeof(bt_core_thread_impl_t));
}

const char *bt_core_thread_result_to_str(bt_core_thread_result_t result)
{
    switch (result) {
        case BT_CORE_THREAD_RESULT_SUCCESS:
            return "Success";
        case BT_CORE_THREAD_RESULT_ERROR_INTERNAL:
            return "Internal";
        default:
            return "Undefined error";
    }
}

void bt_core_thread_fatal_on_error(bt_core_thread_result_t result)
{
    switch (result) {
        case BT_CORE_THREAD_RESULT_SUCCESS:
            return;
        case BT_CORE_THREAD_RESULT_ERROR_INTERNAL:
        default:
            log_fatal("Operation on thread failed: %s", bt_core_thread_result_to_str(result));
    }
}

bt_core_thread_result_t bt_core_thread_create(
    int (*proc)(void *), void *ctx, uint32_t stack_sz, unsigned int priority, bt_core_thread_id_t *thread_id)
{
    return _bt_core_thread_impl.create(proc, ctx, stack_sz, priority, thread_id);
}

bt_core_thread_result_t bt_core_thread_join(bt_core_thread_id_t thread_id, int *result)
{
    return _bt_core_thread_impl.join(thread_id, result);
}

bt_core_thread_result_t bt_core_thread_destroy(bt_core_thread_id_t thread_id)
{
    return _bt_core_thread_impl.destroy(thread_id);
}
