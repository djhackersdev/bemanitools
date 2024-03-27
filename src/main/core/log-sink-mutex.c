#include <windows.h>

#include <stdlib.h>

#include "core/log-sink.h"

#include "util/mem.h"

core_log_sink_t_mutex_ctx {
    core_log_sink_t *child;
    HANDLE mutex;
};

static void
_core_log_sink_mutex_write(void *ctx_, const char *chars, size_t nchars)
{
    core_log_sink_t_mutex_ctx *ctx;

    ctx = (core_log_sink_t_mutex_ctx *) ctx_;

    WaitForSingleObject(ctx->mutex, INFINITE);

    ctx->child->write(ctx->child->ctx, chars, nchars);

    ReleaseMutex(ctx->mutex);
}

static void _core_log_sink_mutex_close(void *ctx_)
{
    core_log_sink_t_mutex_ctx *ctx;

    ctx = (core_log_sink_t_mutex_ctx *) ctx_;

    CloseHandle(ctx->mutex);

    ctx->child->close(ctx->child->ctx);
    free(ctx);
}

void core_log_sink_mutex_open(
    const core_log_sink_t *child_sink, core_log_sink_t *sink)
{
    core_log_sink_t_mutex_ctx *ctx;

    ctx = xmalloc(sizeof(core_log_sink_t_mutex_ctx));

    memcpy(ctx->child, child_sink, sizeof(core_log_sink_t));
    ctx->mutex = CreateMutex(NULL, FALSE, NULL);

    sink->ctx = ctx;
    sink->write = _core_log_sink_mutex_write;
    sink->close = _core_log_sink_mutex_close;
}