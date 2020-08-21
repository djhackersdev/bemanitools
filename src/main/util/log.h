#ifndef UTIL_LOG_H
#define UTIL_LOG_H

#include <stddef.h>
#include <stdlib.h>

#include "bemanitools/glue.h"

#include "util/defs.h"

/* Dynamically retargetable logging system modeled on (and potentially
   integrateable with) the one found in AVS2 */

/* BUILD_MODULE is passed in as a command-line #define by the makefile */

#ifndef LOG_MODULE
#define LOG_MODULE STRINGIFY(BUILD_MODULE)
#endif

#ifndef LOG_SUPPRESS

#define log_misc(...) log_impl_misc(LOG_MODULE, __VA_ARGS__)
#define log_info(...) log_impl_info(LOG_MODULE, __VA_ARGS__)
#define log_warning(...) log_impl_warning(LOG_MODULE, __VA_ARGS__)
#define log_error(...) log_impl_fatal(LOG_MODULE, __VA_ARGS__)

/* This doesn't really belong here, but it's what libavs does so w/e */

#define log_assert(x)                                          \
    do {                                                       \
        if (!(x)) {                                            \
            log_assert_body(__FILE__, __LINE__, __FUNCTION__); \
        }                                                      \
    } while (0)

#else

#define log_misc(...)
#define log_info(...)
#define log_warning(...)
#define log_error(...)
#define log_assert(x) \
    do {              \
        if (!(x)) {   \
            abort();  \
        }             \
    } while (0)

#endif

#define log_fatal(...)                           \
    do {                                         \
        log_impl_fatal(LOG_MODULE, __VA_ARGS__); \
        abort();                                 \
    } while (0)

typedef void (*log_writer_t)(void *ctx, const char *chars, size_t nchars);

extern log_formatter_t log_impl_misc;
extern log_formatter_t log_impl_info;
extern log_formatter_t log_impl_warning;
extern log_formatter_t log_impl_fatal;

void log_assert_body(const char *file, int line, const char *function);
void log_to_external(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal);
void log_to_writer(log_writer_t writer, void *ctx);

void log_set_level(unsigned int new_level);

/* I tried to make this API match the function signature of the AVS log writer
   callback, but then the signature changed and the explicit line breaks
   being passed to that callback went away. So we don't try to track that API
   any more. Launcher defines its own custom writer anyway. */

void log_writer_debug(void *ctx, const char *chars, size_t nchars);
void log_writer_stdout(void *ctx, const char *chars, size_t nchars);
void log_writer_stderr(void *ctx, const char *chars, size_t nchars);
void log_writer_file(void *ctx, const char *chars, size_t nchars);
void log_writer_null(void *ctx, const char *chars, size_t nchars);

#endif
