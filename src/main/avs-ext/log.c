#include "api/core/log.h"

#include "avs-ext/log.h"

#include "iface-core/log.h"

#include "imports/avs.h"

static void _avs_ext_log_core_api_get(bt_core_log_api_t *api)
{
    log_assert(api);

    api->version = 1;

    api->v1.misc = log_body_misc;
    api->v1.info = log_body_info;
    api->v1.warning = log_body_warning;
    api->v1.fatal = log_body_fatal;
}

void avs_ext_log_core_api_set()
{
    bt_core_log_api_t api;

    _avs_ext_log_core_api_get(&api);
    bt_core_log_api_set(&api);
}