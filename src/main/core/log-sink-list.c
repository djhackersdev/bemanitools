#define LOG_MODULE "core-log-sink-list"

#include <stdint.h>
#include <stdlib.h>

#include "core/log-sink-list.h"
#include "core/log-sink.h"

#include "iface-core/log.h"

#include "util/mem.h"

#define MAX_SINKS 8

typedef struct core_log_sink_list {
    core_log_sink_t entries[MAX_SINKS];
    uint8_t num;
} core_log_sink_list_t;

static void
_core_log_sink_list_write(void *ctx, const char *chars, size_t nchars)
{
    core_log_sink_list_t *sink_list;
    int i;

    sink_list = (core_log_sink_list_t *) ctx;

    for (i = 0; i < sink_list->num; i++) {
        sink_list->entries[i].write(sink_list->entries[i].ctx, chars, nchars);
    }
}

static void _core_log_sink_list_close(void *ctx)
{
    core_log_sink_list_t *sink_list;
    int i;

    sink_list = (core_log_sink_list_t *) ctx;

    for (i = 0; i < sink_list->num; i++) {
        sink_list->entries[i].close(sink_list->entries[i].ctx);
    }

    free(sink_list);
}

void core_log_sink_list_open(
    const core_log_sink_t *entry, uint8_t num, core_log_sink_t *sink)
{
    core_log_sink_list_t *sink_list;
    int i;

    log_assert(entry);
    log_assert(sink);

    log_misc("Open: %d", num);

    if (num > MAX_SINKS) {
        abort();
    }

    sink_list = xmalloc(sizeof(core_log_sink_list_t));

    for (i = 0; i < num; i++) {
        sink_list->entries[i].ctx = entry[i].ctx;
        sink_list->entries[i].write = entry[i].write;
        sink_list->entries[i].close = entry[i].close;
    }

    sink_list->num = num;

    sink->ctx = (void *) sink_list;
    sink->write = _core_log_sink_list_write;
    sink->close = _core_log_sink_list_close;
}