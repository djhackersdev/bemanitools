#ifndef PROCMON_PROCMON_H
#define PROCMON_PROCMON_H

#include <stdbool.h>
#include <stdint.h>

#include "util/log.h"

uint32_t procmon_api_version();
void procmon_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal);
void procmon_init();
void procmon_file_mon_enable();
void procmon_module_mon_enable();
void procmon_thread_mon_enable();
void procmon_fini();

#endif