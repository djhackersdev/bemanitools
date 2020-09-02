#pragma once

#include <windows.h>

/**
 * Function signature of shutdown handler.
 */
typedef void (*signal_shutdown_handler_t)();

/**
 * Initialize signal and exception handlers. Exceptions and signals, are logged using the logger
 * to create visibility.
 */
void signal_exception_handler_init();

/**
 * Register a handler function that gets called when the application is supposed to shut down,
 * e.g. on CTRL + C.
 *
 * @param handler Handler function to register.
 */
void signal_register_shutdown_handler(signal_shutdown_handler_t handler);

/**
 * Convert a (windows) exception code to a human readable representation.
 * 
 * @param code Exception code
 * @return Human readable representation of the given exception code
 */ 
const char* signal_exception_code_to_str(DWORD code);