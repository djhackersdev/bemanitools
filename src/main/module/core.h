#ifndef MODULE_CORE_H
#define MODULE_CORE_H

#include "api/core/config.h"
#include "api/core/log.h"
#include "api/core/thread.h"

typedef void (*bt_module_core_config_api_set_t)(
    const bt_core_config_api_t *api);
typedef void (*bt_module_core_log_api_set_t)(const bt_core_log_api_t *api);
typedef void (*bt_module_core_thread_api_set_t)(
    const bt_core_thread_api_t *api);

#endif