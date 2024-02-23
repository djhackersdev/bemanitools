#ifndef CORE_LOG_BT_H
#define CORE_LOG_BT_H

#include "core/log-sink.h"

/**
 * Log API implementation for games/applications without AVS
 */

enum core_log_bt_log_level {
    CORE_LOG_BT_LOG_LEVEL_OFF = 0,
    CORE_LOG_BT_LOG_LEVEL_FATAL = 1,
    CORE_LOG_BT_LOG_LEVEL_WARNING = 2,
    CORE_LOG_BT_LOG_LEVEL_INFO = 3,
    CORE_LOG_BT_LOG_LEVEL_MISC = 4,
};

/**
 * Initialize the logging backend
 *
 * This must be called as early as possible in your application to setup
 * a logging sink according to your needs. Until this is finished, no
 * log output is available.
 *
 * By default, logging is turned off entirely and must be enabled by setting
 * a desired logging level explicitly.
 *
 * @param sink Pointer to a log sink implementation. The caller owns the memory
 *             of this.
 */
void core_log_bt_init(const struct core_log_sink *sink);

/**
 * Set the current logging level. This can be changed at any given time, e.g.
 * to increase/decrease verbosity.
 *
 * @param level The logging level to set.
 */
void core_log_bt_level_set(enum core_log_bt_log_level level);

/**
 * Cleanup the logging backend.
 *
 * Ensure to call this on application exit and cleanup.
 */
void core_log_bt_fini();

/**
 * Implementation of the log API.
 */
void core_log_bt_log_fatal(const char *module, const char *fmt, ...);

/**
 * Implementation of the log API.
 */
void core_log_bt_log_warning(const char *module, const char *fmt, ...);

/**
 * Implementation of the log API.
 */
void core_log_bt_log_info(const char *module, const char *fmt, ...);

/**
 * Implementation of the log API.
 */
void core_log_bt_log_misc(const char *module, const char *fmt, ...);

/**
 * Allow AVS to by-pass the core log API/engine.
 *
 * This function must only be called by AVS in an appropriate log callback
 * function that is passed to avs_boot.
 *
 * AVS has it's own logging engine and manages aspects such as async logging,
 * log levels and decorating log messages.
 *
 * Thus, proper interoperability only requires the writer/sink part to be shared
 * with AVS.
 *
 * @param chars Buffer with text data to write to the configured sinks. The
 *              buffer might contain several log messages separated by newline
 *              characters.
 * @param nchars Number of chars to write to the sink.
 */
void core_log_bt_direct_sink_write(const char *chars, size_t nchars);

#endif