#include <process.h>
#include <windows.h>

#include <stddef.h>
#include <stdint.h>

#include "api/core/thread.h"

#include "core/thread-crt.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "util/defs.h"

struct core_thread_crt_shim_ctx {
    HANDLE barrier;
    int (*proc)(void *);
    void *ctx;
};

static unsigned int STDCALL _core_thread_crt_thread_shim(void *outer_ctx)
{
    struct core_thread_crt_shim_ctx *sctx;
    int (*proc)(void *);
    void *inner_ctx;

    sctx = outer_ctx;

    proc = sctx->proc;
    inner_ctx = sctx->ctx;

    SetEvent(sctx->barrier);

    return proc(inner_ctx);
}

static bt_core_thread_result_t _core_thread_crt_create(
    int (*proc)(void *),
    void *ctx,
    uint32_t stack_sz,
    unsigned int priority,
    bt_core_thread_id_t *thread_id)
{
    struct core_thread_crt_shim_ctx sctx;

    sctx.barrier = CreateEvent(NULL, TRUE, FALSE, NULL);
    sctx.proc = proc;
    sctx.ctx = ctx;

    *thread_id = _beginthreadex(
        NULL, stack_sz, _core_thread_crt_thread_shim, &sctx, 0, NULL);

    WaitForSingleObject(sctx.barrier, INFINITE);
    CloseHandle(sctx.barrier);

    return BT_CORE_THREAD_RESULT_SUCCESS;
}

static bt_core_thread_result_t
_core_thread_crt_join(bt_core_thread_id_t thread_id, int *result)
{
    WaitForSingleObject((HANDLE) (uintptr_t) thread_id, INFINITE);

    if (result) {
        GetExitCodeThread((HANDLE) (uintptr_t) thread_id, (DWORD *) result);
    }

    return BT_CORE_THREAD_RESULT_SUCCESS;
}

static bt_core_thread_result_t
_core_thread_crt_destroy(bt_core_thread_id_t thread_id)
{
    CloseHandle((HANDLE) (uintptr_t) thread_id);

    return BT_CORE_THREAD_RESULT_SUCCESS;
}

static void _core_thread_crt_core_api_get(bt_core_thread_api_t *api)
{
    log_assert(api);

    api->version = 1;

    api->v1.create = _core_thread_crt_create;
    api->v1.join = _core_thread_crt_join;
    api->v1.destroy = _core_thread_crt_destroy;
}

void core_thread_crt_core_api_set()
{
    bt_core_thread_api_t api;

    _core_thread_crt_core_api_get(&api);
    bt_core_thread_api_set(&api);
}