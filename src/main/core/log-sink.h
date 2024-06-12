#ifndef CORE_LOG_SINK_H
#define CORE_LOG_SINK_H

#include <stdint.h>

/**
 * Write function for a log sink implementation.
 *
 * Write the given data to your target output destination.
 *
 * @param ctx Context defined by the implementation when opening the sink.
 * @param chars Buffer with text data to log. This can contain partial data of
 *              a single log line, a full log line terminated by a newline
 *              character or multiple log lines (each terminated by a newline
 *              character).
 * @param nchars Number of characters to write.
 */
typedef void (*core_log_sink_write_t)(
    void *ctx, const char *chars, size_t nchars);

/**
 * Close your log sink and cleanup resources
 *
 * Depending on your implementation, you might want to flush any
 * outstanding/buffered data.
 *
 * @param ctx Context defined by the implementation when opening the sink.
 */
typedef void (*core_log_sink_close_t)(void *ctx);

/**
 * Log sink structure.
 *
 * This must be set-up and populated when opening your log sink implementation.
 * The ctx field contains any arbitrary data that you need for your log sink
 * to operate, e.g. a file handle, additional buffers etc. Make sure these
 * resources are cleaned up upon closing the sink.
 */
typedef struct core_log_sink {
    void *ctx;
    core_log_sink_write_t write;
    core_log_sink_close_t close;
} core_log_sink_t;

#endif