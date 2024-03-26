#include <stdlib.h>

#include "core/thread.h"

static core_thread_impl_t _core_thread_impl;

void core_thread_impl_init(const core_thread_impl_t *impl)
{
    if (!impl || !impl->thread_create || !impl->thread_destroy || !impl->thread_join) {
        abort();
    }

    memcpy(_core_thread_impl, impl, sizeof(core_thread_impl_t));
}

const core_thread_impl_t *core_thread_impl_get()
{
    return &_core_thread_impl;
}

int core_thread_create(
    int (*proc)(void *), void *ctx, uint32_t stack_sz, unsigned int priority)
{
    if (!_core_thread_impl.thread_create) {
        abort();
    }

    return _core_thread_impl.thread_create(proc, ctx, stack_sz, priority);
}

void core_thread_join(int thread_id, int *result)
{
    if (!_core_thread_impl.thread_join) {
        abort();
    }

    _core_thread_impl.thread_join(thread_id, result);
}

void core_thread_destroy(int thread_id)
{
    if (!_core_thread_impl.thread_destroy) {
        abort();
    }

    _core_thread_impl.thread_destroy(thread_id);
}
