#include "core/log.h"

#include "procmon/procmon.h"

void procmon_init(const struct procmon_config *config)
{
    log_assert(config);

    if (config->file_monitor_enable) {
        procmon_file_init();
    }

    if (config->module_monitor_enable) {
        procmon_module_init();
    }

    if (config->thread_monitor_enable) {
        procmon_thread_init();
    }
}

void procmon_fini()
{
    if (_procmon_config.thread_monitor_enable) {
        procmon_thread_fini();
    }

    if (_procmon_config.module_monitor_enable) {
        procmon_module_fini();
    }

    if (_procmon_config.file_monitor_enable) {
        procmon_file_fini();
    }
}