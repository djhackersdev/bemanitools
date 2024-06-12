#ifndef CORE_LOG_SINK_STD_H
#define CORE_LOG_SINK_STD_H

#include <stdbool.h>
#include <stdlib.h>

#include "core/log-sink.h"

/**
 * Create a sink that writes to stdout.
 *
 * @param color If true, messages are colored by log level.
 * @param sink Pointer to allocated memory that receives the opened sink
 */
void core_log_sink_std_out_open(bool color, core_log_sink_t *sink);

/**
 * Create a sink that writes to stderr.
 *
 * @param color If true, messages are colored by log level.
 * @param sink Pointer to allocated memory that receives the opened sink
 */
void core_log_sink_std_err_open(bool color, core_log_sink_t *sink);

#endif