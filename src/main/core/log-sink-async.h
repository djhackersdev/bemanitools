#ifndef CORE_LOG_SINK_ASYNC_H
#define CORE_LOG_SINK_ASYNC_H

#include <stdint.h>
#include <stdlib.h>

#include "core/log-sink.h"

typedef enum core_log_sink_async_overflow_policy {
    // Any to be posted new messages will be dropped, the thread posting the
    // message will not block and wait for space in the queue to be available
    CORE_LOG_SINK_ASYNC_OVERFLOW_POLICY_DISCARD_NEW = 0,
    // Any thread posting messages to the queue will fully block and wait
    // until it acquires space to post the message
    CORE_LOG_SINK_ASYNC_OVERFLOW_POLICY_BLOCK = 1,
} core_log_sink_async_overflow_policy_t;

/**
 * Open a async log sink
 *
 * The sink passes data to log to a separate thread which executes the actual
 * logging of the data
 *
 * @param msg_buffer_size Size for the backend message buffer, ensure this is
 *        at least as large as any buffer sizes the logger uses (i.e. make it
 *        the same size)
 * @param queue_length Length of the queue shared between the threads posting
 *        messages to it and the async background thread consuming them. A
 *        longer queue size increases the memory footprint but allows for
 *        buffering more messages before the overflow policy is applied
 * @param overflow_policy Apply this policy if the threads posting messages
 *        to the queue are facing a full queue (see the overflow policy enum)
 * @param child_sink The target sink the async background will write any
 *        messages consumed from the queue to
 * @param sink Pointer to allocated memory that receives the opened sink
 */
void core_log_sink_async_open(
    // Ensure that buffer size is the same as for the logger
    uint32_t msg_buffer_size,
    uint8_t queue_length,
    core_log_sink_async_overflow_policy_t overflow_policy,
    const core_log_sink_t *child_sink,
    core_log_sink_t *sink);

#endif