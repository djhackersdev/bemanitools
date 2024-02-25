#include <windows.h>

#include <stdlib.h>

#include "core/log-sink.h"

#include "util/mem.h"

struct core_log_sink_std_ctx {
    HANDLE handle;
    bool color;
};

static char _core_log_sink_std_determine_color(const char *str)
{
    /* Add some color to make spotting warnings/errors easier.
        Based on debug output level identifier. */

    /* Avoids colored output on strings like "Windows" */
    if (str[1] != ':') {
        return 15;
    }

    switch (str[0]) {
        /* green */
        case 'M':
            return 10;
        /* blue */
        case 'I':
            return 9;
        /* yellow */
        case 'W':
            return 14;
        /* red */
        case 'F':
            return 12;
        /* default console color */
        default:
            return 15;
    }
}

static size_t _core_log_sink_std_msg_coloring_len(const char *str)
{
    // Expected format example: "I:boot: my log message"

    const char *ptr;
    size_t len;
    int colon_count;

    ptr = str;
    len = 0;
    colon_count = 0;

    while (true) {
        // End of string = invalid log format
        if (*ptr == '\0') {
            return 0;
        }

        if (*ptr == ':') {
            colon_count++;
        }

        if (colon_count == 2) {
            // Skip current colon, next char is a space
            return len + 1;
        }

        len++;
        ptr++;
    }

    return 0;
}

static void
_core_log_sink_std_write(void *ctx_, const char *chars, size_t nchars)
{
    static const size_t timestamp_len = strlen("[----/--/-- --:--:--]");

    struct core_log_sink_std_ctx *ctx;

    char color;
    size_t color_len;
    size_t msg_len;
    const char *msg_start;
    const char *msg_end;
    DWORD written;
    DWORD write_pos;

    ctx = (struct core_log_sink_std_ctx *) ctx_;

    if (ctx->color) {
        write_pos = 0;

        // Support multiple buffered log messages, e.g. from the AVS logging
        // engine
        while (write_pos < nchars) {
            // Expects the AVS timestamp format
            msg_start = chars + timestamp_len + 1; // +1 is the space

            color_len = _core_log_sink_std_msg_coloring_len(msg_start);

            // Check if we could detect which part to color, otherwise just
            // write the whole log message without any coloring logic
            if (color_len > 0) {
                color = _core_log_sink_std_determine_color(msg_start);

                // Timestamp
                WriteConsole(
                    ctx->handle, chars, timestamp_len + 1, &written, NULL);
                write_pos += written;
                chars += written;

                // Log level + module colored
                SetConsoleTextAttribute(ctx->handle, color);
                WriteConsole(ctx->handle, chars, color_len, &written, NULL);
                write_pos += written;
                chars += written;
                SetConsoleTextAttribute(ctx->handle, 15);

                msg_end = strchr(chars, '\n');

                if (msg_end != NULL) {
                    msg_len = msg_end - chars;

                    // Write \n as well
                    msg_len++;

                    // Write actual message non colored
                    WriteConsole(ctx->handle, chars, msg_len, &written, NULL);
                    write_pos += written;
                    chars += written;
                } else {
                    WriteConsole(
                        ctx->handle, chars, nchars - write_pos, &written, NULL);
                    write_pos += written;
                    chars += written;
                }
            } else {
                WriteConsole(
                    ctx->handle,
                    chars + write_pos,
                    nchars - write_pos,
                    &written,
                    NULL);
                write_pos += written;
            }
        }
    } else {
        WriteConsole(ctx->handle, chars, nchars, &written, NULL);
    }
}

static void _core_log_sink_std_close(void *ctx_)
{
    struct core_log_sink_std_ctx *ctx;

    ctx = (struct core_log_sink_std_ctx *) ctx_;

    // Remark: Don't close the ctx->handle, see win API docs

    free(ctx);
}

void core_log_sink_std_out_open(bool color, struct core_log_sink *sink)
{
    struct core_log_sink_std_ctx *ctx;

    ctx = xmalloc(sizeof(struct core_log_sink_std_ctx));

    ctx->handle = GetStdHandle(STD_OUTPUT_HANDLE);
    ctx->color = color;

    sink->ctx = (void *) ctx;
    sink->write = _core_log_sink_std_write;
    sink->close = _core_log_sink_std_close;
}

void core_log_sink_std_err_open(bool color, struct core_log_sink *sink)
{
    struct core_log_sink_std_ctx *ctx;

    ctx = xmalloc(sizeof(struct core_log_sink_std_ctx));

    ctx->handle = GetStdHandle(STD_ERROR_HANDLE);
    ctx->color = color;

    sink->ctx = (void *) ctx;
    sink->write = _core_log_sink_std_write;
    sink->close = _core_log_sink_std_close;
}