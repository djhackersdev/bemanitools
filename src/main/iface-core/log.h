#ifndef BT_CORE_LOG_H
#define BT_CORE_LOG_H

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#include "util/defs.h"

#include "api/core/log.h"

/**
 * The core log API of bemanitools.
 *
 * To a large extent, this reflects the AVS logging API and allows for swapping
 * out the backends with different implementations. Most games should have some
 * version of the AVS API available while some (legacy) games do not. These
 * can use a bemanitools private logging implementation by configuring it
 * in the bootstrapping process.
 */

/* BUILD_MODULE is passed in as a command-line #define by the makefile */

#ifndef LOG_MODULE
#define LOG_MODULE STRINGIFY(BUILD_MODULE)
#endif

/**
 * Log a message on misc level
 *
 * Always use this interface in your application which hides the currently
 * configured implementation.
 *
 * The macro is required to make things work with varargs.
 * The log message is only printed if the log level is set to misc
 *
 * @param fmt printf format string
 * @param ... Additional arguments according to the specified arguments in the
 *            printf format string
 */
#define log_misc_func _bt_core_log_api.v1.misc
#define log_misc(...) log_misc_func(LOG_MODULE, __VA_ARGS__)

/**
 * Log a message on info level
 *
 * Always use this interface in your application which hides the currently
 * configured implementation.
 *
 * The macro is required to make things work with varargs.
 * The log message is only printed if the log level is set to info or lower
 *
 * @param fmt printf format string
 * @param ... Additional arguments according to the specified arguments in the
 *            printf format string
 */
#define log_info(...) _bt_core_log_api.v1.info(LOG_MODULE, __VA_ARGS__)

/**
 * Log a message on warning level
 *
 * Always use this interface in your application which hides the currently
 * configured implementation.
 *
 * The macro is required to make things work with varargs.
 * The log message is only printed if the log level is set to warning or lower
 *
 * @param fmt printf format string
 * @param ... Additional arguments according to the specified arguments in the
 *            printf format string
 */
#define log_warning(...) _bt_core_log_api.v1.warning(LOG_MODULE, __VA_ARGS__)

/**
 * Log a message on fatal level
 *
 * Always use this interface in your application which hides the currently
 * configured implementation.
 *
 * The macro is required to make things work with varargs.
 * The log message is only printed if the log level is set to fatal.
 *
 * This call will also terminate the application.
 *
 * @param fmt printf format string
 * @param ... Additional arguments according to the specified arguments in the
 *            printf format string
 */
// Additional empty log statement to trigger flush on some sinks.
// Otherwise, the last message might get lost
#define log_fatal(...)                                      \
    do {                                                    \
        _bt_core_log_api.v1.fatal(                          \
            LOG_MODULE,                                     \
            "%s:%d: function `%s'",                         \
            __FILE__,                                       \
            __LINE__,                                       \
            __FUNCTION__);                                  \
        _bt_core_log_api.v1.fatal(LOG_MODULE, __VA_ARGS__); \
        _bt_core_log_api.v1.fatal(LOG_MODULE, "");          \
        abort();                                            \
    } while (0)

/**
 * Log a message and terminate the application if given condition fails
 *
 * Always use this interface in your application which hides the currently
 * configured implementation.
 *
 * The macro is required to make things work with varargs.
 *
 * @param x Condition to evaluate. If false, the application terminates
 */
#define log_assert(x)                   \
    do {                                \
        if (!(x)) {                     \
            _bt_core_log_api.v1.fatal(  \
                "assert",               \
                "%s:%d: function `%s'", \
                __FILE__,               \
                __LINE__,               \
                __FUNCTION__);          \
            abort();                    \
        }                               \
    } while (0)

/**
 * Log a message in an exception handler
 *
 * Only use this function in an exception handler, e.g. for stack traces. It
 * logs the message on fatal level but does not terminate.
 *
 * @param fmt printf format string
 * @param ... Additional arguments according to the specified arguments in the
 *            printf format string
 */
#define log_exception_handler(...) \
    _bt_core_log_api.v1.fatal("exception", __VA_ARGS__)

void bt_core_log_api_set(const bt_core_log_api_t *api);
void bt_core_log_api_get(bt_core_log_api_t *api);
void bt_core_log_api_clear();

// Do not use this directly. This is required to work around requiring the
// interface to use var args. Use the macros above to invoke the logging
// functions
extern bt_core_log_api_t _bt_core_log_api;

#endif