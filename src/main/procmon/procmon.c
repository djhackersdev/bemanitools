#define LOG_MODULE "procmon"

#include <stdbool.h>

#include "procmon/file.h"
#include "procmon/module.h"
#include "procmon/thread.h"

#include "util/log.h"

static bool _procmon_file_mon_enabled;
static bool _procmon_module_mon_enabled;
static bool _procmon_thread_mon_enabled;

uint32_t procmon_api_version()
{
    return 0;
}

void procmon_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal)
{
    log_to_external(misc, info, warning, fatal);
}

void procmon_init()
{
    _procmon_file_mon_enabled = false;
    _procmon_module_mon_enabled = false;
    _procmon_thread_mon_enabled = false;

    log_info("init");
}

void procmon_file_mon_enable()
{
    procmon_file_init();
    _procmon_file_mon_enabled = true;
}

void procmon_module_mon_enable()
{
    procmon_module_init();
    _procmon_module_mon_enabled = true;
}

void procmon_thread_mon_enable()
{
    procmon_thread_init();
    _procmon_thread_mon_enabled = true;
}

void procmon_fini()
{
    if (_procmon_file_mon_enabled) {
        procmon_file_fini();
    }

    if (_procmon_module_mon_enabled) {
        procmon_module_fini();
    }

    if (_procmon_thread_mon_enabled) {
        procmon_thread_fini();
    }

    log_info("fini");
}