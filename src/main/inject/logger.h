#include <stdbool.h>

/**
 * Initialize inject's logger backend.
 *
 * This takes care of hooking and merging the different log
 * streams, e.g. inject's local logging and inject's debugger
 * receiving remote logging events.
 *
 * @param log_file_path Path to the file to log to or NULL to
 *                      disable.
 */
bool logger_init(const char *log_file_path);

/**
 * Write a message to the logging backend.
 *
 * This is used by inject's debugger to redirect log messages
 * recevied from the remote process.
 *
 * @param str String to log
 */
void logger_log(const char *str);

/**
 * Shutdown and cleanup the logging backend.
 */
void logger_finit();