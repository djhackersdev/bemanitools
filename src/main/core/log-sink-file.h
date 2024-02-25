#ifndef CORE_LOG_SINK_FILE_H
#define CORE_LOG_SINK_FILE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "core/log-sink.h"

/**
 * Open a log sink writing data to a file
 *
 * @param path Path to the log file to write the log output to
 * @param append If true, then append to an existing file, false to overwrite
 *        any existing file
 * @param rotate If true, rotates an existing log file and creates a new one
 *        for this session
 * @param max_rotations Max number of rotations for the log files
 * @param sink Pointer to allocated memory that receives the opened sink
 */
void core_log_sink_file_open(
    const char *path,
    bool append,
    bool rotate,
    uint8_t max_rotations,
    struct core_log_sink *sink);

#endif