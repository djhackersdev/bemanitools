#define LOG_MODULE "inject-logger"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#include "inject/logger.h"
#include "inject/version.h"

#include "util/log.h"

static FILE *log_file;
static HANDLE log_mutex;

static const char *logger_get_formatted_timestamp(void)
{
    static char buffer[64];
    time_t cur = 0;
    struct tm *tm = NULL;

    cur = time(NULL);
    tm = localtime(&cur);

    strftime(buffer, sizeof(buffer), "[%Y/%m/%d %H:%M:%S] ", tm);

    return buffer;
}

static char logger_console_determine_color(const char *str)
{
    log_assert(str);

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

static size_t logger_msg_coloring_len(const char *str)
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

static void logger_console(
    void *ctx, const char *chars, size_t nchars, const char *timestamp_str)
{
    char color;
    size_t color_len;
    // See "util/log.c", has to align
    char buffer[65536];
    char tmp;

    color_len = logger_msg_coloring_len(chars);

    // Check if we could detect which part to color, otherwise just write the
    // whole log message without any coloring logic
    if (color_len > 0) {
        color = logger_console_determine_color(chars);

        strcpy(buffer, chars);

        // Mask start of log message for coloring
        tmp = buffer[color_len];
        buffer[color_len] = '\0';

        printf("%s", timestamp_str);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
        printf("%s", buffer);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);

        // Write actual message non colored
        buffer[color_len] = tmp;
        printf("%s", buffer + color_len);
    } else {
        printf("%s", chars);
    }
}

static void logger_file(
    void *ctx, const char *chars, size_t nchars, const char *timestamp_str)
{
    if (ctx) {
        fwrite(timestamp_str, 1, strlen(timestamp_str), (FILE *) ctx);
        fwrite(chars, 1, nchars, (FILE *) ctx);
        fflush((FILE *) ctx);
    }
}

static void logger_writer(void *ctx, const char *chars, size_t nchars)
{
    const char *timestamp_str;

    // Different threads logging the same destination, e.g. debugger thread,
    // main thread

    WaitForSingleObject(log_mutex, INFINITE);

    timestamp_str = logger_get_formatted_timestamp();

    logger_console(ctx, chars, nchars, timestamp_str);
    logger_file(ctx, chars, nchars, timestamp_str);

    ReleaseMutex(log_mutex);
}

static void logger_log_header()
{
    log_info(
        "\n"
        "  _        _           _   \n"
        " (_)_ __  (_) ___  ___| |_ \n"
        " | | '_ \\ | |/ _ \\/ __| __|\n"
        " | | | | || |  __/ (__| |_ \n"
        " |_|_| |_|/ |\\___|\\___|\\__|\n"
        "        |__/               ");

    log_info(
        "Inject build date %s, gitrev %s", inject_build_date, inject_gitrev);
}

bool logger_init(const char *log_file_path)
{
    if (log_file_path) {
        log_file = fopen(log_file_path, "w+");
    } else {
        log_file = NULL;
    }

    log_to_writer(logger_writer, log_file);

    logger_log_header();

    if (log_file_path) {
        log_info("Log file: %s", log_file_path);

        if (!log_file) {
            log_warning(
                "ERROR: Opening log file %s failed: %s",
                log_file_path,
                strerror(errno));
            return false;
        }
    }

    log_mutex = CreateMutex(NULL, FALSE, NULL);

    return true;
}

void logger_log(const char *str)
{
    logger_writer(log_file, str, strlen(str));
}

void logger_finit()
{
    log_misc("Logger finit");

    if (log_file) {
        fclose(log_file);
    }

    CloseHandle(log_mutex);
}