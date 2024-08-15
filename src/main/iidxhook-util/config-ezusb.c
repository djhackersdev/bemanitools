#include <stdio.h>
#include <string.h>

#include "cconfig/cconfig-util.h"

#include "core/config-ext.h"

#include "iidxhook-util/config-ezusb.h"

#include "iface-core/log.h"

#include "util/mem.h"
#include "util/str.h"

#define IIDXHOOK_UTIL_CONFIG_EZUSB_API_CALL_MONITORING_KEY \
    "ezusb.api_call_monitoring"
#define IIDXHOOK_UTIL_CONFIG_EZUSB_IO_BOARD_TYPE_KEY "ezusb.io_board_type"

#define IIDXHOOK_UTIL_CONFIG_EZUSB_DEFAULT_API_CALL_MONITORING_VALUE false
#define IIDXHOOK_UTIL_CONFIG_EZUSB_DEFAULT_IO_BOARD_TYPE_VALUE 0

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

void iidxhook_util_config_ezusb_init(struct cconfig *config)
{
    cconfig_util_set_bool(
        config,
        IIDXHOOK_UTIL_CONFIG_EZUSB_API_CALL_MONITORING_KEY,
        IIDXHOOK_UTIL_CONFIG_EZUSB_DEFAULT_API_CALL_MONITORING_VALUE,
        "Enable monitoring of ezusb.dll calls by logging call traces. "
        "Only works on 9th and 10th style!");

    cconfig_util_set_int(
        config,
        IIDXHOOK_UTIL_CONFIG_EZUSB_IO_BOARD_TYPE_KEY,
        IIDXHOOK_UTIL_CONFIG_EZUSB_DEFAULT_IO_BOARD_TYPE_VALUE,
        "Set the type of ezusb IO board. 0 = C02, 1 = D01. "
        "Note: Impacts security settings!");
}

void iidxhook_util_config_ezusb_get(
    struct iidxhook_util_config_ezusb *config_ezusb, struct cconfig *config)
{
    if (!cconfig_util_get_bool(
            config,
            IIDXHOOK_UTIL_CONFIG_EZUSB_API_CALL_MONITORING_KEY,
            &config_ezusb->api_call_monitoring,
            IIDXHOOK_UTIL_CONFIG_EZUSB_DEFAULT_API_CALL_MONITORING_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK_UTIL_CONFIG_EZUSB_API_CALL_MONITORING_KEY,
            IIDXHOOK_UTIL_CONFIG_EZUSB_DEFAULT_API_CALL_MONITORING_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            IIDXHOOK_UTIL_CONFIG_EZUSB_IO_BOARD_TYPE_KEY,
            &config_ezusb->io_board_type,
            IIDXHOOK_UTIL_CONFIG_EZUSB_DEFAULT_IO_BOARD_TYPE_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK_UTIL_CONFIG_EZUSB_IO_BOARD_TYPE_KEY,
            IIDXHOOK_UTIL_CONFIG_EZUSB_DEFAULT_IO_BOARD_TYPE_VALUE);
    }

    if (config_ezusb->io_board_type != 0 && config_ezusb->io_board_type != 1) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK_UTIL_CONFIG_EZUSB_IO_BOARD_TYPE_KEY,
            IIDXHOOK_UTIL_CONFIG_EZUSB_DEFAULT_IO_BOARD_TYPE_VALUE);
        config_ezusb->io_board_type = 0;
    }
}

void iidxhook_util_config_ezusb_get2(
    const bt_core_config_t *config,
    iidxhook_util_config_ezusb_t *config_ezusb)
{
    _iidxhook_util_config_ezusb_board_type_get(config, &config_ezusb->io_board_type);
    bt_core_config_ext_bool_get(config, "ezusb/debug/api_call_monitoring", &config_ezusb->api_call_monitoring);
}