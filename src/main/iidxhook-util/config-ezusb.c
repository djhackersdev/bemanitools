#include <stdio.h>
#include <string.h>

#include "core/config-ext.h"

#include "iidxhook-util/config-ezusb.h"

#include "iface-core/log.h"

#include "util/str.h"

static void _iidxhook_util_config_ezusb_board_type_get(const bt_core_config_t *config, int32_t *board_type)
{
    char tmp[8];

    bt_core_config_ext_str_get(config, "ezusb/type", tmp, sizeof(tmp));

    if (str_eq(tmp, "C02")) {
        *board_type = 0;
    } else if (str_eq(tmp, "D01")) {
        *board_type = 1;
    } else {
        log_fatal("Invalid ezusb board type in configuration: %s", tmp);
    }
}

void iidxhook_util_config_ezusb_get(
    const bt_core_config_t *config,
    iidxhook_util_config_ezusb_t *config_ezusb)
{
    _iidxhook_util_config_ezusb_board_type_get(config, &config_ezusb->io_board_type);
    bt_core_config_ext_bool_get(config, "ezusb/debug/api_call_monitoring", &config_ezusb->api_call_monitoring);
}