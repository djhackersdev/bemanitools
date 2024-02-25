#include <stdlib.h>

#include "core/log.h"
#include "core/thread.h"

core_thread_create_t core_thread_create_impl;
core_thread_join_t core_thread_join_impl;
core_thread_destroy_t core_thread_destroy_impl;

int core_thread_create(
    int (*proc)(void *), void *ctx, uint32_t stack_sz, unsigned int priority)
{
    log_assert(core_thread_create_impl);

    return core_thread_create_impl(proc, ctx, stack_sz, priority);
}

void core_thread_join(int thread_id, int *result)
{
    log_assert(core_thread_join_impl);

    core_thread_join_impl(thread_id, result);
}

void core_thread_destroy(int thread_id)
{
    log_assert(core_thread_destroy_impl);

    core_thread_destroy_impl(thread_id);
}

void core_thread_impl_set(
    core_thread_create_t create,
    core_thread_join_t join,
    core_thread_destroy_t destroy)
{
    if (create == NULL || join == NULL || destroy == NULL) {
        abort();
    }

    core_thread_create_impl = create;
    core_thread_join_impl = join;
    core_thread_destroy_impl = destroy;
}

void core_thread_impl_assign(core_thread_impl_set_t impl_set)
{
    if (core_thread_create_impl == NULL || core_thread_join_impl == NULL ||
        core_thread_destroy_impl == NULL) {
        abort();
    }

    impl_set(
        core_thread_create_impl,
        core_thread_join_impl,
        core_thread_destroy_impl);
}

core_thread_create_t core_thread_create_impl_get()
{
    log_assert(core_thread_create_impl);

    return core_thread_create_impl;
}

core_thread_join_t core_thread_join_impl_get()
{
    log_assert(core_thread_join_impl);

    return core_thread_join_impl;
}

core_thread_destroy_t core_thread_destroy_impl_get()
{
    log_assert(core_thread_destroy_impl);

    return core_thread_destroy_impl;
}
