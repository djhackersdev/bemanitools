#include <windows.h>

#include <stdlib.h>

#include "core/log-sink.h"

#include "util/mem.h"

struct core_log_sink_mutex_ctx {
    struct core_log_sink *child;
    HANDLE mutex;
};

static void
_core_log_sink_mutex_write(void *ctx_, const char *chars, size_t nchars)
{
    struct core_log_sink_mutex_ctx *ctx;

    ctx = (struct core_log_sink_mutex_ctx *) ctx_;

    WaitForSingleObject(ctx->mutex, INFINITE);

    ctx->child->write(ctx->child->ctx, chars, nchars);

    ReleaseMutex(ctx->mutex);
}

static void _core_log_sink_mutex_close(void *ctx_)
{
    struct core_log_sink_mutex_ctx *ctx;

    ctx = (struct core_log_sink_mutex_ctx *) ctx_;

    CloseHandle(ctx->mutex);

    ctx->child->close(ctx->child->ctx);
    free(ctx);
}

void core_log_sink_mutex_open(
    const struct core_log_sink *child_sink, struct core_log_sink *sink)
{
    struct core_log_sink_mutex_ctx *ctx;

    ctx = xmalloc(sizeof(struct core_log_sink_mutex_ctx));

    memcpy(ctx->child, child_sink, sizeof(struct core_log_sink));
    ctx->mutex = CreateMutex(NULL, FALSE, NULL);

    sink->ctx = ctx;
    sink->write = _core_log_sink_mutex_write;
    sink->close = _core_log_sink_mutex_close;
}