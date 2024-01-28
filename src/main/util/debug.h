#ifndef UTIL_DEBUG_H
#define UTIL_DEBUG_H

#include <windows.h>

void debug_init();
void debug_print_stacktrace();
const char *debug_exception_code_to_str(DWORD code);

#endif