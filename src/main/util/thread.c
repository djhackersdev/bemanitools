#include <windows.h>
#include <process.h>

#include <stddef.h>
#include <stdint.h>

#include "util/defs.h"
#include "util/thread.h"

struct shim_ctx {
    HANDLE barrier;
    int (*proc)(void *);
    void *ctx;
};

thread_create_t thread_impl_create = crt_thread_create;
thread_join_t thread_impl_join = crt_thread_join;
thread_destroy_t thread_impl_destroy = crt_thread_destroy;

static unsigned int STDCALL crt_thread_shim(void *outer_ctx)
{
    struct shim_ctx *sctx = outer_ctx;
    int (*proc)(void *);
    void *inner_ctx;

    proc = sctx->proc;
    inner_ctx = sctx->ctx;

    SetEvent(sctx->barrier);

    return proc(inner_ctx);
}

int crt_thread_create(int (*proc)(void *), void *ctx, uint32_t stack_sz,
    unsigned int priority)
{
    struct shim_ctx sctx;
    uintptr_t thread_id;

    sctx.barrier = CreateEvent(NULL, TRUE, FALSE, NULL);
    sctx.proc = proc;
    sctx.ctx = ctx;

    thread_id = _beginthreadex(NULL, stack_sz, crt_thread_shim, &sctx, 0, NULL);

    WaitForSingleObject(sctx.barrier, INFINITE);
    CloseHandle(sctx.barrier);

    return (int) thread_id;
}

void crt_thread_destroy(int thread_id)
{
    CloseHandle((HANDLE) (uintptr_t) thread_id);
}

void crt_thread_join(int thread_id, int *result)
{
    WaitForSingleObject((HANDLE) (uintptr_t) thread_id, INFINITE);

    if (result) {
        GetExitCodeThread((HANDLE) (uintptr_t) thread_id, (DWORD *) result);
    }
}

void thread_api_init(thread_create_t create, thread_join_t join,
        thread_destroy_t destroy)
{
    if (create == NULL || join == NULL || destroy == NULL) {
        abort();
    }

    thread_impl_create = create;
    thread_impl_join = join;
    thread_impl_destroy = destroy;
}

int thread_create(int (*proc)(void *), void *ctx, uint32_t stack_sz,
        unsigned int priority)
{
    return thread_impl_create(proc, ctx, stack_sz, priority);
}

void thread_join(int thread_id, int *result)
{
    thread_impl_join(thread_id, result);
}

void thread_destroy(int thread_id)
{
    thread_impl_destroy(thread_id);
}

