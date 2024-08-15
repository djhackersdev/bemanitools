#define LOG_MODULE "core-log-bt-ext"

#include <stdbool.h>

#include "core/log-bt.h"
#include "core/log-sink-async.h"
#include "core/log-sink-debug.h"
#include "core/log-sink-file.h"
#include "core/log-sink-list.h"
#include "core/log-sink-mutex.h"
#include "core/log-sink-null.h"
#include "core/log-sink-std.h"

// so we can log data dumps of rs232 streams without crashing
#define CORE_LOG_BT_EXT_MSG_BUFFER_SIZE 1024 * 64
// 64 kb * 64 = 4 MB for logging total
#define CORE_LOG_BT_EXT_ASYNC_QUEUE_LENGTH 64

void core_log_bt_ext_init_with_null()
{
    core_log_sink_t sink;

    core_log_sink_null_open(&sink);

    // Size doesn't matter (but must be valid)
    // logger is entirely disabled
    core_log_bt_init(1024, &sink);
    core_log_bt_level_set(CORE_LOG_BT_LOG_LEVEL_OFF);
}

void core_log_bt_ext_init_with_stdout()
{
    core_log_sink_t sink;

    core_log_sink_std_out_open(true, &sink);
    core_log_bt_init(CORE_LOG_BT_EXT_MSG_BUFFER_SIZE, &sink);
}

void core_log_bt_ext_init_with_stderr()
{
    core_log_sink_t sink;

    core_log_sink_std_err_open(true, &sink);
    core_log_bt_init(CORE_LOG_BT_EXT_MSG_BUFFER_SIZE, &sink);
}

void core_log_bt_ext_init_with_debug()
{
    core_log_sink_t sink;

    core_log_sink_debug_open(&sink);
    core_log_bt_init(CORE_LOG_BT_EXT_MSG_BUFFER_SIZE, &sink);
}

void core_log_bt_ext_init_with_file(
    const char *path, bool append, bool rotate, uint8_t max_rotations)
{
    core_log_sink_t sink;

    core_log_sink_file_open(path, append, rotate, max_rotations, &sink);
    core_log_bt_init(CORE_LOG_BT_EXT_MSG_BUFFER_SIZE, &sink);
}

void core_log_bt_ext_init_with_stdout_and_file(
    const char *path, bool append, bool rotate, uint8_t max_rotations)
{
    core_log_sink_t sinks[2];
    core_log_sink_t sink_composed;
    core_log_sink_t sink_mutex;

    core_log_sink_std_out_open(true, &sinks[0]);
    core_log_sink_file_open(path, append, rotate, max_rotations, &sinks[1]);
    core_log_sink_list_open(sinks, 2, &sink_composed);

    core_log_sink_mutex_open(&sink_composed, &sink_mutex);

    core_log_bt_init(CORE_LOG_BT_EXT_MSG_BUFFER_SIZE, &sink_mutex);
}

void core_log_bt_ext_init_with_stderr_and_file(
    const char *path, bool append, bool rotate, uint8_t max_rotations)
{
    core_log_sink_t sinks[2];
    core_log_sink_t sink_composed;
    core_log_sink_t sink_mutex;

    core_log_sink_std_err_open(true, &sinks[0]);
    core_log_sink_file_open(path, append, rotate, max_rotations, &sinks[1]);
    core_log_sink_list_open(sinks, 2, &sink_composed);

    core_log_sink_mutex_open(&sink_composed, &sink_mutex);

    core_log_bt_init(CORE_LOG_BT_EXT_MSG_BUFFER_SIZE, &sink_mutex);
}

void core_log_bt_ext_init_async_with_stderr()
{
    core_log_sink_t sink;
    core_log_sink_t sink_async;

    core_log_sink_std_err_open(true, &sink);

    core_log_sink_async_open(
        CORE_LOG_BT_EXT_MSG_BUFFER_SIZE, 
        CORE_LOG_BT_EXT_ASYNC_QUEUE_LENGTH,
        CORE_LOG_SINK_ASYNC_OVERFLOW_POLICY_DISCARD_NEW,
        &sink, &sink_async);

    core_log_bt_init(CORE_LOG_BT_EXT_MSG_BUFFER_SIZE, &sink_async);
}

void core_log_bt_ext_init_async_with_stderr_and_file(
    const char *path, bool append, bool rotate, uint8_t max_rotations)
{
    core_log_sink_t sinks[2];
    core_log_sink_t sink_composed;
    core_log_sink_t sink_async;

    core_log_sink_std_err_open(true, &sinks[0]);
    core_log_sink_file_open(path, append, rotate, max_rotations, &sinks[1]);
    core_log_sink_list_open(sinks, 2, &sink_composed);

    core_log_sink_async_open(
        CORE_LOG_BT_EXT_MSG_BUFFER_SIZE, 
        CORE_LOG_BT_EXT_ASYNC_QUEUE_LENGTH,
        CORE_LOG_SINK_ASYNC_OVERFLOW_POLICY_DISCARD_NEW,
        &sink_composed, &sink_async);

    core_log_bt_init(CORE_LOG_BT_EXT_MSG_BUFFER_SIZE, &sink_async);
}