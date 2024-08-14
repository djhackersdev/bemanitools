#ifndef UTIL_DEBUG_H
#define UTIL_DEBUG_H

#include <windows.h>

#include "core/log.h"

void debug_init(core_log_message_t exception_msg);
void debug_print_stacktrace();
const char *debug_exception_code_to_str(DWORD code);

#endif