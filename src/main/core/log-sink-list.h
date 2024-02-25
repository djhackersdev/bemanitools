#ifndef CORE_LOG_SINK_LIST_H
#define CORE_LOG_SINK_LIST_H

#include <stdint.h>
#include <stdlib.h>

#include "core/log-sink.h"

/**
 * Combine multiple log sinks into a list of sinks.
 *
 * Upon invoking a list sink, all sinks contained within the list are
 * being invoked in the configured order.
 *
 * @param entry A pointer to allocated memory with a sequence of opened sinks
 *              that you want to add to the list. Ownership of these sinks
 *              is transferred, i.e. closing the list sink closes its children.
 * @param num The number of elements in the sequence of opened sinks pointed to.
 * @param sink Pointer to allocated memory that receives the opened sink
 */
void core_log_sink_list_open(
    const struct core_log_sink *entry, uint8_t num, struct core_log_sink *sink);

#endif