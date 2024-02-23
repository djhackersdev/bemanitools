#ifndef CORE_LOG_H
#define CORE_LOG_H

#include <stddef.h>
#include <stdlib.h>

#include "util/defs.h"

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
#define log_misc(...) _core_log_misc_impl(LOG_MODULE, __VA_ARGS__)

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
#define log_info(...) _core_log_info_impl(LOG_MODULE, __VA_ARGS__)

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
#define log_warning(...) _core_log_warning_impl(LOG_MODULE, __VA_ARGS__)

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
        _core_log_fatal_impl(LOG_MODULE, __VA_ARGS__); \
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
            _core_log_fatal_impl(       \
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
    _core_log_fatal_impl("exception", __VA_ARGS__)

typedef void (*core_log_message_t)(const char *module, const char *fmt, ...);

typedef void (*core_log_impl_set_t)(
    core_log_message_t misc,
    core_log_message_t info,
    core_log_message_t warning,
    core_log_message_t fatal);

/**
 * Configure the log API implementations
 *
 * Advised to do this as early in your application/library module as possible
 * as calls to the getter functions below will return the currently configured
 * implementations.
 *
 * @param misc Pointer to a function implementing logging on misc level
 * @param info Pointer to a function implementing logging on info level
 * @param warning Pointer to a function implementing logging on warning level
 * @param fatal Pointer to a function implementing logging on fatal level
 */
void core_log_impl_set(
    core_log_message_t misc,
    core_log_message_t info,
    core_log_message_t warning,
    core_log_message_t fatal);

/**
 * Supporting function to inject/assign the currently set implementation
 * with the given setter function.
 *
 * @param impl_set Setter function to call with the currently configured log
 *        function implementations
 */
void core_log_impl_assign(core_log_impl_set_t impl_set);

/**
 * Get the currently configured implementation of the misc level log function
 *
 * @return Pointer to the currently configured implementation of the function
 */
core_log_message_t core_log_misc_impl_get();

/**
 * Get the currently configured implementation of the info level log function
 *
 * @return Pointer to the currently configured implementation of the function
 */
core_log_message_t core_log_info_impl_get();

/**
 * Get the currently configured implementation of the warning level log function
 *
 * @return Pointer to the currently configured implementation of the function
 */
core_log_message_t core_log_warning_impl_get();

/**
 * Get the currently configured implementation of the fatal level log function
 *
 * @return Pointer to the currently configured implementation of the function
 */
core_log_message_t core_log_fatal_impl_get();

// Do not use these directly.
// These are only here to allow usage in the macros above.
extern core_log_message_t _core_log_misc_impl;
extern core_log_message_t _core_log_info_impl;
extern core_log_message_t _core_log_warning_impl;
extern core_log_message_t _core_log_fatal_impl;

#endif