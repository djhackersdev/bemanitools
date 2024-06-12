#define LOG_MODULE "core-log-sink-async"

#include <stdlib.h>

#include "core/log-sink.h"

#include "iface-core/log.h"

static void
_core_log_sink_file_write(void *ctx, const char *chars, size_t nchars)
{
    // TODO
}

static void _core_log_sink_file_close(void *ctx)
{
    // TODO
}

void core_log_sink_async_open(core_log_sink_t *sink)
{
    log_assert(sink);

    // TODO

    log_misc("Open");

    sink->ctx = NULL;
    sink->write = _core_log_sink_file_write;
    sink->close = _core_log_sink_file_close;
}