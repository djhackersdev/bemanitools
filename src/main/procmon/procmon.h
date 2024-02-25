#ifndef PROCMON_PROCMON_H
#define PROCMON_PROCMON_H

#include <stdbool.h>
#include <stdint.h>

#include "core/log.h"

uint32_t procmon_api_version();
void procmon_set_loggers(
    core_log_message_t misc,
    core_log_message_t info,
    core_log_message_t warning,
    core_log_message_t fatal);
void procmon_init();
void procmon_file_mon_enable();
void procmon_module_mon_enable();
void procmon_thread_mon_enable();
void procmon_fini();

#endif