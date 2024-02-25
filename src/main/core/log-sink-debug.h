#ifndef CORE_LOG_SINK_DEBUG_H
#define CORE_LOG_SINK_DEBUG_H

#include <stdlib.h>

#include "core/log-sink.h"

/**
 * Open a log sink that uses OutputDebugStr
 *
 * @param sink Pointer to allocated memory that receives the opened sink
 */
void core_log_sink_debug_open(struct core_log_sink *sink);

#endif