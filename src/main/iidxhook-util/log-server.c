#include <windows.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "hook/table.h"

#include "iidxhook-util/log-server.h"

#include "imports/avs.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/str.h"
#include "util/thread.h"

static int log_thread_proc(void *ctx);
static void
log_post(char level, const char *module, const char *fmt, va_list ap);
static void log_post_misc(const char *module, const char *fmt, ...);
static void log_post_info(const char *module, const char *fmt, ...);
static void log_post_warning(const char *module, const char *fmt, ...);
static void log_post_fatal(const char *module, const char *fmt, ...);

static int log_thread_id;
static HANDLE log_rv_producer;
static HANDLE log_rv_consumer;
static char log_rv_level;
static const char *log_rv_module;
static char log_rv_buffer[8192];

void log_server_init(void)
{
    HANDLE ready;

    log_rv_producer = CreateSemaphore(NULL, 1, 1, NULL);
    log_rv_consumer = CreateSemaphore(NULL, 0, 1, NULL);
    ready = CreateEvent(NULL, TRUE, FALSE, NULL);

    log_to_external(
        log_post_misc, log_post_info, log_post_warning, log_post_fatal);

    log_thread_id = avs_thread_create(log_thread_proc, ready, 16384, 0);

    if (WaitForSingleObject(ready, INFINITE)) {
        // can't do any logging here, yet.
        fprintf(
            stderr,
            "ERROR log_server_init: WaitForSingleObject failed: %08x",
            (unsigned int) GetLastError());
        return;
    }

    CloseHandle(ready);

    log_misc("Started log server thread");
}

static int log_thread_proc(void *ctx)
{
    bool run;

    SetEvent((HANDLE) ctx);

    run = true;

    log_body_misc(LOG_MODULE, "Log server thread is running");

    while (run) {
        if (WaitForSingleObject(log_rv_consumer, INFINITE)) {
            log_fatal(
                "WaitForSingleObject failed: %08x",
                (unsigned int) GetLastError());
        }

        switch (log_rv_level) {
            case '\0':
                run = false;

                break;

            case 'M':
                log_body_misc(log_rv_module, "%s", log_rv_buffer);

                break;

            case 'I':
                log_body_info(log_rv_module, "%s", log_rv_buffer);

                break;

            case 'W':
                log_body_warning(log_rv_module, "%s", log_rv_buffer);

                break;

            case 'F':
                log_body_fatal(log_rv_module, "%s", log_rv_buffer);

                break;
        }

        if (!ReleaseSemaphore(log_rv_producer, 1, NULL)) {
            log_fatal(
                "ReleaseSemaphore failed: %08x", (unsigned int) GetLastError());
        }
    }

    log_body_misc(LOG_MODULE, "Log server thread is exiting");

    return 0;
}

static void
log_post(char level, const char *module, const char *fmt, va_list ap)
{
    // TODO test if this addresses performance issues and stuttering
    // TODO measure time how long waiting takes here?
    if (WaitForSingleObject(log_rv_producer, INFINITE)) {
        return;
    }

    log_rv_level = level;
    log_rv_module = module;
    str_vformat(log_rv_buffer, sizeof(log_rv_buffer), fmt, ap);

    ReleaseSemaphore(log_rv_consumer, 1, NULL);
}

#define LOG_POST_IMPL(name, level)                             \
    static void name(const char *module, const char *fmt, ...) \
    {                                                          \
        va_list ap;                                            \
                                                               \
        va_start(ap, fmt);                                     \
        log_post(level, module, fmt, ap);                      \
        va_end(ap);                                            \
    }

LOG_POST_IMPL(log_post_misc, 'M')
LOG_POST_IMPL(log_post_info, 'I')
LOG_POST_IMPL(log_post_warning, 'W')
LOG_POST_IMPL(log_post_fatal, 'F')

void log_server_fini(void)
{
    int thread_result;

    log_misc("Stopping log server thread");

    WaitForSingleObject(log_rv_producer, INFINITE);
    log_rv_level = '\0';
    ReleaseSemaphore(log_rv_consumer, 1, NULL);

    avs_thread_join(log_thread_id, &thread_result);
    avs_thread_destroy(log_thread_id);

    CloseHandle(log_rv_producer);
    CloseHandle(log_rv_consumer);

    log_rv_producer = NULL;
    log_rv_consumer = NULL;
}
