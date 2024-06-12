#ifndef CORE_LOG_SINK_MUTEX_H
#define CORE_LOG_SINK_MUTEX_H

#include <stdlib.h>

#include "core/log-sink.h"

/**
 * Create a sink that surrounds another sink with a mutex.
 *
 * Use this to make other sink implementations thread-safe.
 *
 * @param child_sink Another opened sink to surround with the mutex. Ownership
 *                   of the sink is transferred, i.e. closing the mutex sink
 *                   also closes the wrapped child sink.
 * @param sink Pointer to allocated memory that receives the opened sink
 */
void core_log_sink_mutex_open(
    const core_log_sink_t *child_sink, core_log_sink_t *sink);

#endif