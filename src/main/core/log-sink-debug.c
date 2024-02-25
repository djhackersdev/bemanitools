#include <debugapi.h>

#include <stdlib.h>

#include "core/log-sink.h"

static void
_core_log_sink_debug_write(void *ctx, const char *chars, size_t nchars)
{
    OutputDebugStringA(chars);
}

static void _core_log_sink_debug_close(void *ctx)
{
    // noop
}

void core_log_sink_debug_open(struct core_log_sink *sink)
{
    sink->ctx = NULL;
    sink->write = _core_log_sink_debug_write;
    sink->close = _core_log_sink_debug_close;
}