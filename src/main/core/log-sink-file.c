#define LOG_MODULE "core-log-sink-file"

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "core/log-sink.h"

#include "iface-core/log.h"

#include "util/fs.h"
#include "util/str.h"

static void _core_log_sink_file_rotate(const char *path, uint8_t max_rotations)
{
    uint8_t i;
    char rotate_file[MAX_PATH];
    char rotate_file_next[MAX_PATH];
    char version[8];
    char version_next[8];

    for (i = max_rotations; i > 0; i++) {
        str_cpy(rotate_file, sizeof(rotate_file), path);
        str_cpy(rotate_file_next, sizeof(rotate_file_next), path);

        if (i - 1 != 0) {
            sprintf(version, ".%d", i);
        } else {
            memset(version, 0, sizeof(version));
        }

        sprintf(version_next, ".%d", i);

        str_cat(rotate_file, sizeof(rotate_file), version);
        str_cat(rotate_file_next, sizeof(rotate_file_next), version_next);

        if (path_exists(rotate_file)) {
            CopyFile(rotate_file, rotate_file_next, FALSE);
        }
    }
}

static void
_core_log_sink_file_write(void *ctx, const char *chars, size_t nchars)
{
    FILE *file;

    file = (FILE *) ctx;

    fwrite(chars, 1, nchars, file);
}

static void _core_log_sink_file_close(void *ctx)
{
    FILE *file;

    file = (FILE *) ctx;

    fflush(file);
    fclose(file);
}

void core_log_sink_file_open(
    const char *path,
    bool append,
    bool rotate,
    uint8_t max_rotations,
    core_log_sink_t *sink)
{
    FILE *file;

    log_assert(path);
    log_assert(sink);

    log_info(
        "Open: %s, append %s, rotate %d, max_rotations %d",
        path,
        append,
        rotate,
        max_rotations);

    if (rotate) {
        _core_log_sink_file_rotate(path, max_rotations);

        // Appending doesn't matter when file is rotated anyway
        file = fopen(path, "w+");
    } else {
        if (append) {
            file = fopen(path, "a+");
        } else {
            file = fopen(path, "w+");
        }
    }

    if (!file) {
        log_fatal("Cannot open log file: %s", path);
    }

    sink->ctx = (void *) file;
    sink->write = _core_log_sink_file_write;
    sink->close = _core_log_sink_file_close;
}