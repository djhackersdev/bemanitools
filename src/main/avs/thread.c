#include "avs/thread.h"

#include "core/log.h"

#include "imports/avs.h"

static core_thread_result_t _avs_thread_create(
    int (*proc)(void *), void *ctx, uint32_t stack_sz, unsigned int priority, core_thread_id_t *thread_id)
{
    *thread_id = avs_thread_create(proc, ctx, stack_sz, priority);

    return CORE_THREAD_RESULT_SUCCESS;
}

static core_thread_result_t _avs_thread_join(core_thread_id_t thread_id, int *result)
{
    avs_thread_join(thread_id, result);

    return CORE_THREAD_RESULT_SUCCESS;
}

static core_thread_result_t _avs_thread_destroy(core_thread_id_t thread_id)
{
    avs_thread_destroy(thread_id);

    return CORE_THREAD_RESULT_SUCCESS;
}

void avs_thread_impl_get(core_thread_impl_t *impl)
{
    log_assert(impl);

    impl->create = _avs_thread_create;
    impl->join = _avs_thread_join;
    impl->destroy = _avs_thread_destroy;
}