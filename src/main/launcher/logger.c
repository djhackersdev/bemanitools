#define LOG_MODULE "launcher-logger"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#include "launcher/logger.h"
#include "launcher/version.h"

#include "util/fs.h"
#include "util/log.h"
#include "util/mem.h"

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

static void logger_console_avs(const char *chars)
{
    char color;
    size_t color_len;
    // See "util/log.c", has to align
    char buffer[65536];
    char tmp;
    const char *timestamp;
    const char *msg;

    static const size_t timestamp_len = strlen("[----/--/-- --:--:--]");

    timestamp = chars;
    msg = timestamp + timestamp_len + 1; // +1 is the space

    color_len = logger_msg_coloring_len(msg);

    // Check if we could detect which part to color, otherwise just write the
    // whole log message without any coloring logic
    if (color_len > 0) {
        color = logger_console_determine_color(msg);

        strcpy(buffer, msg);

        // Mask start of log message for coloring
        tmp = buffer[color_len];
        buffer[color_len] = '\0';

        printf("%.21s ", timestamp);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
        printf("%s", buffer);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);

        // Write actual message non colored
        buffer[color_len] = tmp;
        printf("%s\n", buffer + color_len);
    } else {
        printf("%s\n", chars);
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

static void logger_file_avs(const char *chars, size_t len)
{
    if (log_file) {
        fwrite(chars, 1, len, log_file);
        fflush(log_file);
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
        " .__                             .__                   \n"
        " |  | _____   __ __  ____   ____ |  |__   ___________  \n"
        " |  | \\__  \\ |  |  \\/    \\_/ ___\\|  |  \\_/ __ \\_  __ \\ \n"
        " |  |__/ __ \\|  |  /   |  \\  \\___|   Y  \\  ___/|  | \\/ \n"
        " |____(____  /____/|___|  /\\___  >___|  /\\___  >__|    \n"
        "          \\/           \\/     \\/     \\/     \\/       ");

    log_info(
        "launcher build date %s, gitrev %s",
        launcher_build_date,
        launcher_gitrev);
}

void logger_early_init(const char *log_file_path)
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
            log_fatal(
                "ERROR: Opening log file %s failed: %s",
                log_file_path,
                strerror(errno));
        }
    }

    log_mutex = CreateMutex(NULL, FALSE, NULL);
}

void logger_init(
    const char *filename,
    bool enable_console,
    bool enable_file,
    bool rotate_file,
    bool append_file,
    uint16_t count_file)
{
    // Remark: very basic implementation for now, logger needs a proper cleanup
    // anyway before implementing more features such as rotation

    if (enable_file) {
        if (log_file) {
            // Log file stitching of early log output
            fseek(log_file, 0, SEEK_END);
            size_t file_size = ftell(log_file);
            fseek(log_file, 0, SEEK_SET);

            void *buffer = xmalloc(file_size);

            fread(buffer, file_size, 1, log_file);

            fclose(log_file);

            log_file = fopen(filename, "w+");

            fwrite(buffer, file_size, 1, log_file);
            fflush(log_file);
            free(buffer);
        }
    } else {
        if (log_file) {
            fclose(log_file);
            log_file = NULL;
        }
    }
}

void logger_level_set(enum logger_level level)
{
    enum log_level internal_level;

    switch (level) {
        case LOGGER_LEVEL_OFF:
        case LOGGER_LEVEL_FATAL:
            internal_level = LOG_LEVEL_FATAL;
            break;

        case LOGGER_LEVEL_WARNING:
            internal_level = LOG_LEVEL_WARNING;
            break;

        case LOGGER_LEVEL_DEFAULT:
        case LOGGER_LEVEL_INFO:
            internal_level = LOG_LEVEL_INFO;
            break;

        case LOGGER_LEVEL_MISC:
        case LOGGER_LEVEL_ALL:
            internal_level = LOG_LEVEL_MISC;
            break;

        default:
            internal_level = LOG_LEVEL_FATAL;
            log_assert(false);
    }

    log_set_level(internal_level);
}

void logger_log_avs_log_message(char *str, size_t len)
{
    bool use_crlf;

#if AVS_VERSION >= 1500
    use_crlf = true;
#else
    use_crlf = false;
#endif

    // Different threads logging the same destination, e.g. debugger thread,
    // main thread

    WaitForSingleObject(log_mutex, INFINITE);

    // Write to file first, tokenizing trashes the string
    logger_file_avs(str, len);

    // The character stream provided here can contain multiple lines as AVS
    // manages buffered writes to the logger backend. We need to split them
    // up for the logging backend to allow for further enhancements

    char *line = strtok(str, use_crlf ? "\r\n" : "\n");

    while (line) {
        logger_console_avs(line);
        line = strtok(NULL, use_crlf ? "\r\n" : "\n");
    }

    ReleaseMutex(log_mutex);
}

void logger_finit()
{
    log_misc("Logger finit");

    if (log_file) {
        fflush(log_file);
        fclose(log_file);
    }

    CloseHandle(log_mutex);
}