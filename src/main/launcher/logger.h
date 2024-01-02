#ifndef LAUNCHER_LOGGER_H
#define LAUNCHER_LOGGER_H

#include <stdbool.h>
#include <stdint.h>

enum logger_level {
    LOGGER_LEVEL_OFF = 0,
    LOGGER_LEVEL_FATAL = 1,
    LOGGER_LEVEL_WARNING = 2,
    LOGGER_LEVEL_INFO = 3,
    LOGGER_LEVEL_MISC = 4,
    LOGGER_LEVEL_ALL = 5,
    LOGGER_LEVEL_DEFAULT = 6
};

void logger_early_init(const char *log_file_path);

void logger_init(
    const char *filename,
    bool enable_console,
    bool enable_file,
    bool rotate_file,
    bool append_file,
    uint16_t count_file);

void logger_level_set(enum logger_level level);

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

#endif