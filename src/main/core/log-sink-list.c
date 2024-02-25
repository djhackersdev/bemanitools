#include <stdint.h>
#include <stdlib.h>

#include "core/log-sink-list.h"
#include "core/log-sink.h"

#include "util/mem.h"

#define MAX_SINKS 8

struct core_log_sink_list {
    struct core_log_sink entries[MAX_SINKS];
    uint8_t num;
};

static void
_core_log_sink_list_write(void *ctx, const char *chars, size_t nchars)
{
    struct core_log_sink_list *sink_list;
    int i;

    sink_list = (struct core_log_sink_list *) ctx;

    for (i = 0; i < sink_list->num; i++) {
        sink_list->entries[i].write(sink_list->entries[i].ctx, chars, nchars);
    }
}

static void _core_log_sink_list_close(void *ctx)
{
    struct core_log_sink_list *sink_list;
    int i;

    sink_list = (struct core_log_sink_list *) ctx;

    for (i = 0; i < sink_list->num; i++) {
        sink_list->entries[i].close(sink_list->entries[i].ctx);
    }

    free(sink_list);
}

void core_log_sink_list_open(
    const struct core_log_sink *entry, uint8_t num, struct core_log_sink *sink)
{
    struct core_log_sink_list *sink_list;
    int i;

    if (num > MAX_SINKS) {
        abort();
    }

    sink_list = xmalloc(sizeof(struct core_log_sink_list));

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