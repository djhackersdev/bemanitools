#ifndef CORE_LOG_SINK_ASYNC_H
#define CORE_LOG_SINK_ASYNC_H

#include <stdint.h>
#include <stdlib.h>

#include "core/log-sink.h"

/**
 * Open a async log sink
 *
 * The sink passes data to log to a separate thread which executes the actual
 * logging of the data.
 *
 * @param sink Pointer to allocated memory that receives the opened sink
 */
void core_log_sink_async_open(struct core_log_sink *sink);

#endif