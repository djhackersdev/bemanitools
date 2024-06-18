#define LOG_MODULE "core-log-sink-async"

#include <windows.h>

#include <stdatomic.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#include "core/log-bt.h"
#include "core/log-sink.h"
#include "core/log-sink-async.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "util/mem.h"
#include "util/str.h"

#define CORE_LOG_SINK_ASYNC_TIMESTAMP_SIZE_MAX 64
// That's plenty for the few messages with fairly bounded message size we got
#define CORE_LOG_SINK_ASYNC_DIRECT_LOG_BUFFER_SIZE 1024 * 4

#define log_safe_direct_misc(ctx, ...) \
    _core_log_sink_async_direct_log(ctx, CORE_LOG_BT_LOG_LEVEL_MISC, __VA_ARGS__)
#define log_safe_direct_info(ctx, ...) \
    _core_log_sink_async_direct_log(ctx, CORE_LOG_BT_LOG_LEVEL_INFO, __VA_ARGS__)
#define log_safe_direct_warning(ctx, ...) \
    _core_log_sink_async_direct_log(ctx, CORE_LOG_BT_LOG_LEVEL_WARNING, __VA_ARGS__)
#define log_safe_direct_fatal(ctx, ...) \
    do { \
        _core_log_sink_async_direct_log(ctx, CORE_LOG_BT_LOG_LEVEL_FATAL, __VA_ARGS__); \
        ctx->target_sink.close(ctx->target_sink.ctx); \
        abort(); \
    } while (false)    

typedef struct core_log_sink_async_queue_elem {
    char *buffer;
    size_t len;
} core_log_sink_async_queue_elem_t;

typedef struct core_log_sink_async_queue {
    size_t len_public;
    size_t len_internal;
    core_log_sink_async_queue_elem_t *elem;
    size_t pos_front;
    size_t pos_back;
    HANDLE mutex;
} core_log_sink_async_queue_t;

typedef struct core_log_sink_async {
    uint32_t msg_buffer_size;
    core_log_sink_async_overflow_policy_t overflow_policy;
    core_log_sink_t target_sink;

    core_log_sink_async_queue_t *queue;

    HANDLE semaphore_producer;
    HANDLE semaphore_consumer;

    bt_core_thread_id_t thread_id;
    _Atomic(bool) thread_proc_loop;
    _Atomic(bool) thread_proc_running;
} core_log_sink_async_t;

static core_log_sink_async_queue_t *_core_log_sink_async_queue_alloc(size_t buffer_size, size_t len)
{
    size_t len_internal;
    core_log_sink_async_queue_t *queue;

    queue = xmalloc(sizeof(core_log_sink_async_queue_t));

    // Actual queue length is always len + 1 because we need one "dead spot"
    // that isn't used when the queue is considered full. Otherwise, there
    // is no way to distinguish an empty from a full queue with this setup
    // Example:
    // - Queue that can store 4 elements
    // - It is empty when front_pos == back_pos
    // - Filling up the queue with 4 elements back_pos increments 4 times
    // - Applying wrap-around with modulo, back_pos points to first index
    //   again which is front_pos if no elements were consumed thus far
    len_internal = len + 1;

    queue->elem = xmalloc(sizeof(core_log_sink_async_queue_elem_t) * len_internal);

    for (size_t i = 0; i < len_internal; i++) {
        queue->elem[i].buffer = xmalloc(sizeof(char) * buffer_size);
        queue->elem[i].len = 0;
    }

    queue->len_public = len;
    queue->len_internal = len_internal;
    queue->pos_front = 0;
    queue->pos_back = 0;
    queue->mutex = CreateMutex(NULL, FALSE, NULL);

    return queue;
}

static bool _core_log_sink_async_queue_push(core_log_sink_async_queue_t *queue, const char *buffer, size_t len)
{
    size_t back;
    size_t front;
    size_t index;

    back = queue->pos_back;
    front = queue->pos_front;

    // Optimistic check outside of lock to reduce lock contention
    // Queue full
    if (back + 1 % queue->len_internal == front % queue->len_internal) {
        return false;
    }

    WaitForSingleObject(queue->mutex, INFINITE);

    // Repeat operation within lock to get consistent state
    if (queue->pos_back + 1 % queue->len_internal == queue->pos_front % queue->len_internal) {
        ReleaseMutex(queue->mutex);
        return false;
    }

    index = (queue->pos_back + 1) % queue->len_internal;

    memcpy(queue->elem[index].buffer, buffer, len);
    queue->elem[index].len = len;

    queue->pos_back++;

    ReleaseMutex(queue->mutex);

    return true;
}

static bool _core_log_sink_async_queue_pop(core_log_sink_async_queue_t *queue, char *buffer, size_t *len)
{
    size_t front;
    size_t back;
    size_t index;

    // Be "optimistic" and do this outside the lock to reduce lock contention
    // with producers
    front = queue->pos_front;
    back = queue->pos_back;

    // Empty
    if (front % queue->len_internal == back % queue->len_internal) {
        return false;
    }

    WaitForSingleObject(queue->mutex, INFINITE);

    // Another check within the lock
    if (queue->pos_front % queue->len_internal == queue->pos_back % queue->len_internal) {
        ReleaseMutex(queue->mutex);
        return false;
    }

    index = queue->pos_front % queue->len_internal;

    memcpy(buffer, queue->elem[index].buffer, queue->elem[index].len);
    *len = queue->elem[index].len;

    queue->pos_front++;

    ReleaseMutex(queue->mutex);

    return true;
}

static void _core_log_sink_async_queue_free(core_log_sink_async_queue_t *queue)
{
    for (size_t i = 0; i < queue->len_internal; i++) {
        free(queue->elem[i].buffer);
    }

    free(queue->elem);
    CloseHandle(queue->mutex);
    free(queue);
}

// Consumer and producer must write directly to the sink in case there is
// an issue with the async sink itself. They can't go through the logging
// system because that's hooked up to this sink
// Copy-paste of log-bt with minor tweaks to have log levels and timestamps available
static void _core_log_sink_async_direct_log(
    core_log_sink_async_t *ctx,
    enum core_log_bt_log_level level,
    const char *fmt,
    ...)
{
    static const char chars[] = "FFWIM";

    va_list args;
    char timestamp[CORE_LOG_SINK_ASYNC_TIMESTAMP_SIZE_MAX];
    char msg[CORE_LOG_SINK_ASYNC_DIRECT_LOG_BUFFER_SIZE];
    char line[CORE_LOG_SINK_ASYNC_DIRECT_LOG_BUFFER_SIZE];
    int result;

    time_t curtime;
    struct tm *tm;

    va_start(args, fmt);

    curtime = 0;
    tm = NULL;

    curtime = time(NULL);
    tm = localtime(&curtime);

    strftime(timestamp, sizeof(timestamp), "[%Y/%m/%d %H:%M:%S]", tm);

    str_vformat(msg, sizeof(msg), fmt, args);

    va_end(args);

    result = str_format(
        line,
        sizeof(line),
        "%s %c:%s: %s\n",
        timestamp,
        chars[level],
        LOG_MODULE,
        msg);

    ctx->target_sink.write(ctx->target_sink.ctx, line, result);
}

static int _core_log_sink_async_thread_proc(void *ctx_)
{
    core_log_sink_async_t *ctx;
    char *buffer;
    size_t len;
    bool is_empty;

    ctx = (core_log_sink_async_t*) ctx_;

    // Own private buffer, pop from queue copies data to it
    buffer = xmalloc(sizeof(char) * ctx->msg_buffer_size);
    len = 0;

    atomic_store_explicit(&ctx->thread_proc_running, true, memory_order_seq_cst);

    log_safe_direct_misc(ctx, "Thread running");

    while (atomic_load_explicit(&ctx->thread_proc_loop, memory_order_seq_cst)) {
        // Wait until any producer signals that there is data in the queue
        // Use this instead of polling the queue to save CPU resources
        if (WaitForSingleObject(ctx->semaphore_producer, INFINITE)) {
            // Need to write directly to sink as the logging system might be broken
            log_safe_direct_fatal(ctx, "WaitForSingleObject failed: %08x", (unsigned int) GetLastError());
        }

        is_empty = !_core_log_sink_async_queue_pop(ctx->queue, buffer, &len);

        ReleaseSemaphore(ctx->semaphore_consumer, 1, NULL);

        // Execute potentially expensive IO writing the data outside of critical section
        // to avoid lock contention
        if (!is_empty) {
            ctx->target_sink.write(ctx->target_sink.ctx, buffer, len);
        } else {
            log_safe_direct_warning(ctx, "Popping from queue failed, queue is empty");
        }
    }

    free(buffer);

    atomic_store_explicit(&ctx->thread_proc_running, false, memory_order_seq_cst);

    log_safe_direct_misc(ctx, "Thread running");

    return 0;
}

static void
_core_log_sink_async_write(void *ctx_, const char *chars, size_t nchars)
{
    core_log_sink_async_t *ctx;
    DWORD result;
    bool is_full;

    ctx = (core_log_sink_async_t*) ctx_;

    // "Try acquire" a semaphore to avoid blocking the caller if not desired
    // (see overflow policy)
    result = WaitForSingleObject(ctx->semaphore_consumer, 0);

    if (result == WAIT_TIMEOUT) {
        switch (ctx->overflow_policy) {
            case CORE_LOG_SINK_ASYNC_OVERFLOW_POLICY_DISCARD_NEW:
                return;
            case CORE_LOG_SINK_ASYNC_OVERFLOW_POLICY_BLOCK:
                if (WaitForSingleObject(ctx->semaphore_consumer, INFINITE)) {
                    log_safe_direct_fatal(ctx, "WaitForSingleObject failed: %08x", (unsigned int) GetLastError());
                }

                break;
            default:
                log_safe_direct_fatal(ctx, "Illegal state for overflow policy: %d", ctx->overflow_policy);
        }
    } else if (result != WAIT_OBJECT_0) {
        log_safe_direct_fatal(ctx, "WaitForSingleObject 2 failed: %08x", (unsigned int) GetLastError());
    }

    // This copies the data to the buffers allocated with the queue which
    // is a non-blocking operation at this point. The semaphores are
    // orchestrating multiple threads to ensure that only if there is
    // capacity in the queue threads will get here
    is_full = !_core_log_sink_async_queue_push(ctx->queue, chars, nchars);

    if (is_full) {
        log_safe_direct_warning(ctx, "Invalid state, pushing log message to queue failed, queue full");
    } else {
        ReleaseSemaphore(ctx->semaphore_producer, 1, NULL);
    }
}

static void _core_log_sink_async_close(void *ctx_)
{
    core_log_sink_async_t *ctx;
    bt_core_thread_result_t result;

    ctx = (core_log_sink_async_t*) ctx_;

    log_safe_direct_info(ctx, "Shutting down logger thread and waiting for it to finish...");

    atomic_store_explicit(&ctx->thread_proc_loop, false, memory_order_seq_cst);

    result = bt_core_thread_join(ctx->thread_id, NULL);

    if (BT_CORE_THREAD_RESULT_IS_ERROR(BT_CORE_THREAD_RESULT_SUCCESS)) {
        log_safe_direct_fatal(ctx, "Joining logger thread failed");
    } else {
        log_safe_direct_info(ctx, "Logger thread finished");
    }

    result = bt_core_thread_destroy(ctx->thread_id);

    if (BT_CORE_THREAD_RESULT_IS_ERROR(result)) {
        log_safe_direct_fatal(ctx, "Destroying logger thread failed");
    }

    CloseHandle(ctx->semaphore_producer);
    CloseHandle(ctx->semaphore_consumer);

    _core_log_sink_async_queue_free(ctx->queue);

    // Last message before closing the child sink
    log_safe_direct_misc(ctx, "Shutting down done");

    ctx->target_sink.close(ctx->target_sink.ctx);

    free(ctx);
}

void core_log_sink_async_open(
    uint32_t msg_buffer_size,
    uint8_t queue_length,
    core_log_sink_async_overflow_policy_t overflow_policy,
    const core_log_sink_t *child_sink,
    core_log_sink_t *sink)
{
    core_log_sink_async_t *ctx;
    bt_core_thread_result_t result;

    // Sanity check for sizes we consider too much or too less
    log_assert(msg_buffer_size > 1024); // 1 kB
    log_assert(msg_buffer_size <= 1024 * 1024); // 1 MB
    // Limits max space for buffers to 128 MB which is plenty for excessive
    // debug logging tasks of stuff like IO dumps, if necessary
    log_assert(queue_length < 128);
    log_assert(child_sink);
    log_assert(sink);

    ctx = xmalloc(sizeof(core_log_sink_async_t));

    ctx->msg_buffer_size = msg_buffer_size;
    ctx->overflow_policy = overflow_policy;

    ctx->target_sink.ctx = child_sink->ctx;
    ctx->target_sink.write = child_sink->write;
    ctx->target_sink.close = child_sink->close;

    ctx->queue = _core_log_sink_async_queue_alloc(msg_buffer_size, queue_length);

    ctx->semaphore_producer = CreateSemaphore(NULL, 0, queue_length, NULL);
    ctx->semaphore_consumer = CreateSemaphore(NULL, queue_length, queue_length, NULL);

    atomic_store_explicit(&ctx->thread_proc_loop, true, memory_order_seq_cst);
    atomic_store_explicit(&ctx->thread_proc_running, false, memory_order_seq_cst);

    result = bt_core_thread_create(_core_log_sink_async_thread_proc, ctx, 16384, 0, &ctx->thread_id);
    bt_core_thread_fatal_on_error(result);

    log_misc("Waiting for thread to start...");

    // wait until thread is running
    while (!atomic_load_explicit(&ctx->thread_proc_running, memory_order_seq_cst)) {
        Sleep(10);
    }

    sink->ctx = ctx;
    sink->write = _core_log_sink_async_write;
    sink->close = _core_log_sink_async_close;

    log_misc("Open: msg_buffer_size %d, queue_length %d, overflow_policy %d", msg_buffer_size, queue_length, overflow_policy);
}