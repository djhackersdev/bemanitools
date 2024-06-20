#ifndef CORE_LOG_SINK_NULL_H
#define CORE_LOG_SINK_NULL_H

#include <stdlib.h>

#include "core/log-sink.h"

/**
 * Create a null/dummy sink.
 *
 * Use this to disable any logging entirely.
 *
 * @param sink Pointer to allocated memory that receives the opened sink
 */
void core_log_sink_null_open(struct core_log_sink *sink);

#endif