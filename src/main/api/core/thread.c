#include "btapi/core/thread.h"

#include "core/thread.h"

static bt_core_thread_result_t _api_core_thread_create(int (*proc)(void *), void *ctx, uint32_t stack_sz, unsigned int priority, bt_core_thread_id_t *thread_id)
{
    return (bt_core_thread_result_t) core_thread_create(proc, ctx, stack_sz, priority, thread_id);
}

static bt_core_thread_result_t _api_core_thread_join(bt_core_thread_id_t thread_id, int *result)
{
    return (bt_core_thread_result_t) core_thread_join(thread_id, result);
}

static bt_core_thread_result_t _api_core_thread_destroy(bt_core_thread_id_t thread_id)
{
    return (bt_core_thread_result_t) core_thread_destroy(thread_id);
}

void api_core_thread_impl_get(bt_core_thread_impl_t *impl)
{
    log_assert(impl);

    impl->create = _api_core_thread_create;
    impl->join = _api_core_thread_join;
    impl->destroy = _api_core_thread_destroy;
}