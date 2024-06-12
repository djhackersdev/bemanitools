#define LOG_MODULE "core-log-bt"

#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#include "iface-core/log.h"

#include "main/core/log-bt.h"
#include "main/core/log-sink.h"

#include "util/mem.h"
#include "util/str.h"

/* 64k so we can log data dumps of rs232 without crashing */
#define CORE_LOG_MSG_SIZE_MAX 65536
#define CORE_LOG_TIMESTAMP_SIZE_MAX 64

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
    char msg[CORE_LOG_MSG_SIZE_MAX];
    char line[CORE_LOG_MSG_SIZE_MAX];
    int result;

    time_t curtime;
    struct tm *tm;

    curtime = 0;
    tm = NULL;

    curtime = time(NULL);
    tm = localtime(&curtime);

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

void core_log_bt_init(const core_log_sink_t *sink)
{
    log_assert(sink);

    log_misc("Init");

    _core_log_bt_sink = xmalloc(sizeof(core_log_sink_t));
    memcpy(_core_log_bt_sink, sink, sizeof(core_log_sink_t));

    _core_log_bt_log_level = CORE_LOG_BT_LOG_LEVEL_OFF;
}

void core_log_bt_reinit(const core_log_sink_t *sink)
{
    log_assert(_core_log_bt_sink);

    log_misc("Re-init");

    _core_log_bt_sink->close(_core_log_bt_sink->ctx);

    free(_core_log_bt_sink);

    _core_log_bt_sink = xmalloc(sizeof(core_log_sink_t));
    memcpy(_core_log_bt_sink, sink, sizeof(core_log_sink_t));

    _core_log_bt_log_level = CORE_LOG_BT_LOG_LEVEL_OFF;
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
