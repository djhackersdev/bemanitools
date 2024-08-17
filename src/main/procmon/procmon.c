#include "iface-core/log.h"

#include "procmon/file.h"
#include "procmon/module.h"
#include "procmon/procmon.h"
#include "procmon/thread.h"

static bool _procmon_file_monitor_enabled;
static bool _procmon_module_monitor_enabled;
static bool _procmon_thread_monitor_enabled;

void procmon_init(const procmon_config_t *config)
{
    log_assert(config);

    _procmon_file_monitor_enabled = config->file_enable;
    _procmon_module_monitor_enabled = config->module_enable;
    _procmon_thread_monitor_enabled = config->thread_enable;

    if (_procmon_file_monitor_enabled) {
        procmon_file_init();
    }

    if (_procmon_module_monitor_enabled) {
        procmon_module_init();
    }

    if (_procmon_thread_monitor_enabled) {
        procmon_thread_init();
    }
}

void procmon_fini()
{
    if (_procmon_thread_monitor_enabled) {
        procmon_thread_fini();
    }

    if (_procmon_module_monitor_enabled) {
        procmon_module_fini();
    }

    if (_procmon_file_monitor_enabled) {
        procmon_file_fini();
    }
}