#define LOG_MODULE "procmon"

#include <stdbool.h>
#include <stdint.h>

#include "core/log.h"

#include "procmon/file.h"
#include "procmon/module.h"
#include "procmon/procmon.h"
#include "procmon/thread.h"

static bool _procmon_file_mon_enabled;
static bool _procmon_module_mon_enabled;
static bool _procmon_thread_mon_enabled;

uint32_t procmon_api_version()
{
    return 0;
}

void procmon_set_loggers(
    core_log_message_t misc,
    core_log_message_t info,
    core_log_message_t warning,
    core_log_message_t fatal)
{
    core_log_impl_set(misc, info, warning, fatal);
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