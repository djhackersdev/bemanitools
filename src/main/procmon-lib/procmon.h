#ifndef PROCMON_LIB_PROCMON_H
#define PROCMON_LIB_PROCMON_H

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

#include "util/log.h"

typedef uint32_t (*procmon_api_version_t)();
typedef void (*procmon_set_loggers_t)(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal);
typedef void (*procmon_file_mon_enable_t)();
typedef void (*procmon_module_mon_enable_t)();
typedef void (*procmon_thread_mon_enable_t)();
typedef void (*procmon_init_t)();

struct procmon {
    HMODULE module;
    procmon_api_version_t api_version;
    procmon_set_loggers_t set_loggers;
    procmon_file_mon_enable_t file_mon_enable;
    procmon_module_mon_enable_t module_mon_enable;
    procmon_thread_mon_enable_t thread_mon_enable;
    procmon_init_t init;
};

void procmon_init(struct procmon *procmon);
bool procmon_available();
void procmon_load(struct procmon *procmon);
void procmon_free(struct procmon *procmon);

#endif