#include <process.h>
#include <windows.h>

#include <stddef.h>
#include <stdint.h>

#include "core/thread-crt.h"
#include "core/thread.h"

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

static core_thread_result_t _core_thread_crt_create(
    int (*proc)(void *), void *ctx, uint32_t stack_sz, unsigned int priority, core_thread_id_t *thread_id)
{
    struct core_thread_crt_shim_ctx sctx;

    sctx.barrier = CreateEvent(NULL, TRUE, FALSE, NULL);
    sctx.proc = proc;
    sctx.ctx = ctx;

    *thread_id = _beginthreadex(NULL, stack_sz, _core_thread_crt_thread_shim, &sctx, 0, NULL);

    WaitForSingleObject(sctx.barrier, INFINITE);
    CloseHandle(sctx.barrier);

    return CORE_THREAD_RESULT_SUCCESS;
}

static core_thread_result_t _core_thread_crt_join(core_thread_id_t thread_id, int *result)
{
    WaitForSingleObject((HANDLE) (uintptr_t) thread_id, INFINITE);

    if (result) {
        GetExitCodeThread((HANDLE) (uintptr_t) thread_id, (DWORD *) result);
    }

    return CORE_THREAD_RESULT_SUCCESS;
}

static core_thread_result_t _core_thread_crt_destroy(core_thread_id_t thread_id)
{
    CloseHandle((HANDLE) (uintptr_t) thread_id);

    return CORE_THREAD_RESULT_SUCCESS;
}

void core_thread_crt_impl_get(core_thread_impl_t *impl)
{
    log_assert(impl);

    impl->create = _core_thread_crt_create;
    impl->join = _core_thread_crt_join;
    impl->destroy = _core_thread_crt_destroy;
}