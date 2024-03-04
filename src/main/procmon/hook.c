#define LOG_MODULE "procmon-hook"

#include "core/log-bt-ext.h"
#include "core/log.h"
#include "core/thread-crt-ext.h"
#include "core/thread.h"

#include "hook.h"

#include "procmon/procmon.h"

void btapi_hook_thread_impl_set(
    btapi_thread_create_t create,
    btapi_thread_join_t join,
    btapi_thread_destroy_t destroy)
{
    core_thread_impl_set(create, join, destroy);
}

void btapi_hook_log_impl_set(
    btapi_log_formatter_t misc,
    btapi_log_formatter_t info,
    btapi_log_formatter_t warning,
    btapi_log_formatter_t fatal)
{
    core_log_impl_set(misc, info, warning, fatal);
}

bool btapi_hook_init(struct property_node *config)
{
    log_assert(config);

    procmon_init();

    procmon_file_mon_enable();
    procmon_module_mon_enable();
    procmon_thread_mon_enable();    

    return true;
}

void btapi_hook_fini()
{
    procmon_fini();
}
