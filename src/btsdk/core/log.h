#ifndef BTSDK_CORE_LOG_H
#define BTSDK_CORE_LOG_H

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#include "btapi/core/log.h"

#include "util/defs.h"

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
#define log_misc(...) _bt_core_log_impl.misc(LOG_MODULE, __VA_ARGS__)

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
#define log_info(...) _bt_core_log_impl.info(LOG_MODULE, __VA_ARGS__)

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
#define log_warning(...) _bt_core_log_impl.warning(LOG_MODULE, __VA_ARGS__)

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
#define log_fatal(...)                                 \
    do {                                               \
        _bt_core_log_impl.fatal(LOG_MODULE, __VA_ARGS__); \
        abort();                                       \
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
            _bt_core_log_impl.fatal(       \
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
    _bt_core_log_impl.fatal("exception", __VA_ARGS__)

void bt_core_log_impl_set(const bt_core_log_impl_t *impl);

// Do not use this directly. This is required to work around requiring the interface to use
// var args. Use the macros above to invoke the logging functions
extern bt_core_log_impl_t _bt_core_log_impl;

#endif