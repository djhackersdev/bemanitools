#include <stdbool.h>

/**
 * Initialize launcher's early logger backend (before AVS's logger is available).
 *
 * @param log_file_path Path to the file to log to or NULL to
 *                      disable.
 */
bool logger_init(const char *log_file_path);

/**
 * Write a log message from AVS to the logging backend.
 *
 * @param str String to log
 * @param len Total length of the log string
 */
void logger_log_avs_log_message(char *str, size_t len);

/**
 * Shutdown and cleanup the logging backend.
 */
void logger_finit();