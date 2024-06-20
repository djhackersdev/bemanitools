#include <stdlib.h>

#include "core/log-sink.h"

static void
_core_log_sink_null_write(void *ctx, const char *chars, size_t nchars)
{
    // noop
}

static void _core_log_sink_null_close(void *ctx)
{
    // noop
}

void core_log_sink_null_open(struct core_log_sink *sink)
{
    sink->ctx = NULL;
    sink->write = _core_log_sink_null_write;
    sink->close = _core_log_sink_null_close;
}