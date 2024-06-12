#include <stdbool.h>

#include "core/log-bt.h"
#include "core/log-sink-debug.h"
#include "core/log-sink-file.h"
#include "core/log-sink-list.h"
#include "core/log-sink-mutex.h"
#include "core/log-sink-std.h"
#include "core/log.h"

void core_log_bt_ext_impl_set()
{
    core_log_impl_set(
        core_log_bt_log_misc,
        core_log_bt_log_info,
        core_log_bt_log_warning,
        core_log_bt_log_fatal);
}

void core_log_bt_ext_init_with_stdout()
{
    struct core_log_sink sink;

    core_log_sink_std_out_open(true, &sink);
    core_log_bt_init(&sink);
}

void core_log_bt_ext_init_with_stderr()
{
    struct core_log_sink sink;

    core_log_sink_std_err_open(true, &sink);
    core_log_bt_init(&sink);
}

void core_log_bt_ext_init_with_debug()
{
    struct core_log_sink sink;

    core_log_sink_debug_open(&sink);
    core_log_bt_init(&sink);
}

void core_log_bt_ext_init_with_file(
    const char *path, bool append, bool rotate, uint8_t max_rotations)
{
    struct core_log_sink sink;

    core_log_sink_file_open(path, append, rotate, max_rotations, &sink);
    core_log_bt_init(&sink);
}

void core_log_bt_ext_init_with_stdout_and_file(
    const char *path, bool append, bool rotate, uint8_t max_rotations)
{
    struct core_log_sink sinks[2];
    struct core_log_sink sink_composed;
    struct core_log_sink sink_mutex;

    core_log_sink_std_out_open(true, &sinks[0]);
    core_log_sink_file_open(path, append, rotate, max_rotations, &sinks[1]);
    core_log_sink_list_open(sinks, 2, &sink_composed);

    core_log_sink_mutex_open(&sink_composed, &sink_mutex);

    core_log_bt_init(&sink_mutex);
}