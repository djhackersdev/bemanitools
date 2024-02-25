#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#include "core/log-bt.h"
#include "core/log-sink.h"
#include "core/log.h"

#include "util/mem.h"
#include "util/str.h"

static enum core_log_bt_log_level _core_log_bt_log_level;
static struct core_log_sink *_core_log_bt_sink;

static void _core_log_bt_vformat_write(
    enum core_log_bt_log_level level,
    const char *module,
    const char *fmt,
    va_list ap)
{
    static const char chars[] = "FFWIM";

    char timestamp[64];
    /* 64k so we can log data dumps of rs232 without crashing */
    char msg[65536];
    char line[65536];
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

void core_log_bt_init(const struct core_log_sink *sink)
{
    if (sink == NULL) {
        abort();
    }

    _core_log_bt_sink = xmalloc(sizeof(struct core_log_sink));
    memcpy(_core_log_bt_sink, sink, sizeof(struct core_log_sink));

    _core_log_bt_log_level = CORE_LOG_BT_LOG_LEVEL_OFF;
}

void core_log_bt_level_set(enum core_log_bt_log_level level)
{
    _core_log_bt_log_level = level;
}

void core_log_bt_fini()
{
    log_assert(_core_log_bt_sink);

    _core_log_bt_sink->close(_core_log_bt_sink->ctx);

    free(_core_log_bt_sink);
}

void core_log_bt_log_fatal(const char *module, const char *fmt, ...)
{
    va_list ap;

    if (_core_log_bt_log_level >= CORE_LOG_BT_LOG_LEVEL_FATAL) {
        va_start(ap, fmt);
        _core_log_bt_vformat_write(
            CORE_LOG_BT_LOG_LEVEL_FATAL, module, fmt, ap);
        va_end(ap);
    }
}

void core_log_bt_log_warning(const char *module, const char *fmt, ...)
{
    va_list ap;

    if (_core_log_bt_log_level >= CORE_LOG_BT_LOG_LEVEL_WARNING) {
        va_start(ap, fmt);
        _core_log_bt_vformat_write(
            CORE_LOG_BT_LOG_LEVEL_WARNING, module, fmt, ap);
        va_end(ap);
    }
}

void core_log_bt_log_info(const char *module, const char *fmt, ...)
{
    va_list ap;

    if (_core_log_bt_log_level >= CORE_LOG_BT_LOG_LEVEL_INFO) {
        va_start(ap, fmt);
        _core_log_bt_vformat_write(CORE_LOG_BT_LOG_LEVEL_INFO, module, fmt, ap);
        va_end(ap);
    }
}

void core_log_bt_log_misc(const char *module, const char *fmt, ...)
{
    va_list ap;

    if (_core_log_bt_log_level >= CORE_LOG_BT_LOG_LEVEL_MISC) {
        va_start(ap, fmt);
        _core_log_bt_vformat_write(CORE_LOG_BT_LOG_LEVEL_MISC, module, fmt, ap);
        va_end(ap);
    }
}

void core_log_bt_direct_sink_write(const char *chars, size_t nchars)
{
    _core_log_bt_sink->write(_core_log_bt_sink->ctx, chars, nchars);
}