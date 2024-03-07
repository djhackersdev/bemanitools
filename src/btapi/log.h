#ifndef BTAPI_LOG_H
#define BTAPI_LOG_H

#include <stdint.h>

/* An AVS-style logger function. Comes in four flavors: misc, info, warning,
   and fatal, with increasing severity. Fatal loggers do not return, they
   abort the running process after writing their message to the log.

   "module" is an arbitrary short string identifying the source of the log
   message. The name of the calling DLL is a good default choice for this
   string, although you might want to identify a module within your DLL here
   instead.

   "fmt" is a printf-style format string. Depending on the context in which
   your DLL is running you might end up calling a logger function exported
   from libavs, which has its own printf implementation (including a number of
   proprietary extensions), so don't use any overly exotic formats. */

// typedef void (*btapi_log_formatter_t)(const char *module, const char *fmt, ...)
//     LOG_CHECK_FMT;

void btapi_log_misc(const char *module, const char *fmt, ...);
void btapi_log_info(const char *module, const char *fmt, ...);
void btapi_log_warning(const char *module, const char *fmt, ...);
void btapi_log_fatal(const char *module, const char *fmt, ...);

#endif
