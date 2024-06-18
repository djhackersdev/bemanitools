#define LOG_MODULE "core-log-bt"

#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#include "iface-core/log.h"

#include "main/core/log-bt.h"
#include "main/core/log-sink.h"

#include "util/mem.h"
#include "util/str.h"

#define CORE_LOG_TIMESTAMP_SIZE_MAX 64

static uint32_t _core_log_bt_msg_buffer_size;
static enum core_log_bt_log_level _core_log_bt_log_level;
static core_log_sink_t *_core_log_bt_sink;

static void _core_log_bt_vformat_timestamp_log_level_write(
    enum core_log_bt_log_level level,
    const char *module,
    const char *fmt,
    va_list ap)
{
    static const char chars[] = "FFWIM";

    char timestamp[CORE_LOG_TIMESTAMP_SIZE_MAX];
    char msg[_core_log_bt_msg_buffer_size];
    char line[_core_log_bt_msg_buffer_size];
    int result;

    time_t curtime;
    struct tm *tm;

    curtime = 0;
    tm = NULL;

    curtime = time(NULL);
    tm = localtime(&curtime);

    // TODO truncate messages if too large and emit log warning message this happened

    strftime(timestamp, sizeof(timestamp), "[%Y/%m/%d %H:%M:%S]", tm);

    str_vformat(msg, sizeof(msg), fmt, ap);

    result = str_format(
        line,
        sizeof(line),
        "%s %c:%s: %s\n",
        timestamp,
        chars[level],
        module,
        msg);

    _core_log_bt_sink->write(_core_log_bt_sink->ctx, line, result);
}

static void _core_log_bt_log_misc(const char *module, const char *fmt, ...)
{
    va_list args;

    // Cut off logging as early as possible if the logging level isn't configured
    // This saves processing time that might have performance impact on the
    // calling thread
    if (_core_log_bt_log_level >= CORE_LOG_BT_LOG_LEVEL_MISC) {
        va_start(args, fmt);
        _core_log_bt_vformat_timestamp_log_level_write(
            CORE_LOG_BT_LOG_LEVEL_MISC, module, fmt, args);
        va_end(args);
    }
}

static void _core_log_bt_log_info(const char *module, const char *fmt, ...)
{
    va_list args;

    // Cut off logging as early as possible if the logging level isn't configured
    // This saves processing time that might have performance impact on the
    // calling thread
    if (_core_log_bt_log_level >= CORE_LOG_BT_LOG_LEVEL_INFO) {
        va_start(args, fmt);
        _core_log_bt_vformat_timestamp_log_level_write(
            CORE_LOG_BT_LOG_LEVEL_INFO, module, fmt, args);
        va_end(args);
    }
}

static void _core_log_bt_log_warning(const char *module, const char *fmt, ...)
{
    va_list args;

    // Cut off logging as early as possible if the logging level isn't configured
    // This saves processing time that might have performance impact on the
    // calling thread
    if (_core_log_bt_log_level >= CORE_LOG_BT_LOG_LEVEL_WARNING) {
        va_start(args, fmt);
        _core_log_bt_vformat_timestamp_log_level_write(
            CORE_LOG_BT_LOG_LEVEL_WARNING, module, fmt, args);
        va_end(args);
    }
}

static void _core_log_bt_log_fatal(const char *module, const char *fmt, ...)
{
    va_list args;

    // Cut off logging as early as possible if the logging level isn't configured
    // This saves processing time that might have performance impact on the
    // calling thread
    if (_core_log_bt_log_level >= CORE_LOG_BT_LOG_LEVEL_FATAL) {
        va_start(args, fmt);
        _core_log_bt_vformat_timestamp_log_level_write(
            CORE_LOG_BT_LOG_LEVEL_FATAL, module, fmt, args);
        va_end(args);
    }
}

void core_log_bt_core_api_get(bt_core_log_api_t *api)
{
    log_assert(api);

    api->version = 1;

    api->v1.misc = _core_log_bt_log_misc;
    api->v1.info = _core_log_bt_log_info;
    api->v1.warning = _core_log_bt_log_warning;
    api->v1.fatal = _core_log_bt_log_fatal;
}

void core_log_bt_core_api_set()
{
    bt_core_log_api_t api;

    core_log_bt_core_api_get(&api);
    bt_core_log_api_set(&api);
}

void core_log_bt_init(
    size_t msg_buffer_size,
    const core_log_sink_t *sink)
{
    // Sanity check for sizes we consider too much or too less
    log_assert(msg_buffer_size > 1024); // 1 kB
    log_assert(msg_buffer_size <= 1024 * 1024); // 1 MB
    log_assert(sink);

    _core_log_bt_msg_buffer_size = msg_buffer_size;

    _core_log_bt_sink = xmalloc(sizeof(core_log_sink_t));
    memcpy(_core_log_bt_sink, sink, sizeof(core_log_sink_t));

    _core_log_bt_log_level = CORE_LOG_BT_LOG_LEVEL_OFF;

    log_misc("Init: msg_buffer_size %d", msg_buffer_size);
}

void core_log_bt_reinit(
    size_t msg_buffer_size,
    const core_log_sink_t *sink)
{
    log_assert(_core_log_bt_sink);

    log_misc("Re-init");

    core_log_bt_fini();
    core_log_bt_init(msg_buffer_size, sink);
}

void core_log_bt_level_set(enum core_log_bt_log_level level)
{
    log_misc("Log level change from %d to %d", _core_log_bt_log_level, level);

    _core_log_bt_log_level = level;
}

void core_log_bt_fini()
{
    log_assert(_core_log_bt_sink);

    log_misc("Fini");

    _core_log_bt_sink->close(_core_log_bt_sink->ctx);

    free(_core_log_bt_sink);
}

void core_log_bt_direct_sink_write(const char *chars, size_t nchars)
{
    _core_log_bt_sink->write(_core_log_bt_sink->ctx, chars, nchars);
}
