#define LOG_MODULE "bt-core-log"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "iface-core/log.h"

#define BT_CORE_LOG_ASSERT_IMPLEMENTED(func, name)                 \
    if (!func) {                                                   \
        log_fatal("Function %s not implemented", STRINGIFY(name)); \
    }

bt_core_log_api_t _bt_core_log_api;

static bool _bt_core_log_api_is_valid()
{
    return _bt_core_log_api.version > 0;
}

void bt_core_log_api_set(const bt_core_log_api_t *api)
{
    log_assert(api);

    if (_bt_core_log_api_is_valid()) {
        log_info("Re-initialize");
    }

    if (api->version == 1) {
        BT_CORE_LOG_ASSERT_IMPLEMENTED(api->v1.misc, misc);
        BT_CORE_LOG_ASSERT_IMPLEMENTED(api->v1.info, info);
        BT_CORE_LOG_ASSERT_IMPLEMENTED(api->v1.warning, warning);
        BT_CORE_LOG_ASSERT_IMPLEMENTED(api->v1.fatal, fatal);

        memcpy(&_bt_core_log_api, api, sizeof(bt_core_log_api_t));

        log_misc("api v1 set");
    } else {
        log_fatal("Unsupported API version: %d", api->version);
    }
}

void bt_core_log_api_get(bt_core_log_api_t *api)
{
    log_assert(api);
    log_assert(_bt_core_log_api_is_valid());

    memcpy(api, &_bt_core_log_api, sizeof(bt_core_log_api_t));
}

void bt_core_log_api_clear()
{
    log_assert(_bt_core_log_api_is_valid());

    memset(&_bt_core_log_api, 0, sizeof(bt_core_log_api_t));

    log_misc("api cleared");
}