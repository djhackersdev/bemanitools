#include "api/core/thread.h"
#include "api/core/log.h"

#include "avs-ext/thread.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "imports/avs.h"

static bt_core_thread_result_t _avs_ext_thread_create(
    int (*proc)(void *),
    void *ctx,
    uint32_t stack_sz,
    unsigned int priority,
    bt_core_thread_id_t *thread_id)
{
    *thread_id = avs_thread_create(proc, ctx, stack_sz, priority);

    return BT_CORE_THREAD_RESULT_SUCCESS;
}

static bt_core_thread_result_t
_avs_ext_thread_join(bt_core_thread_id_t thread_id, int *result)
{
    avs_thread_join(thread_id, result);

    return BT_CORE_THREAD_RESULT_SUCCESS;
}

static bt_core_thread_result_t
_avs_ext_thread_destroy(bt_core_thread_id_t thread_id)
{
    avs_thread_destroy(thread_id);

    return BT_CORE_THREAD_RESULT_SUCCESS;
}

static void _avs_ext_thread_core_api_get(bt_core_thread_api_t *api)
{
    log_assert(api);

    api->version = 1;

    api->v1.create = _avs_ext_thread_create;
    api->v1.join = _avs_ext_thread_join;
    api->v1.destroy = _avs_ext_thread_destroy;
}

void avs_ext_thread_core_api_set()
{
    bt_core_thread_api_t api;

    _avs_ext_thread_core_api_get(&api);
    bt_core_thread_api_set(&api);
}