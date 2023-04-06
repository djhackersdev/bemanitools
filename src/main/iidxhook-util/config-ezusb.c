#include <stdio.h>
#include <string.h>

#include "cconfig/cconfig-util.h"

#include "iidxhook-util/config-ezusb.h"

#include "util/log.h"
#include "util/mem.h"

#define IIDXHOOK_UTIL_CONFIG_EZUSB_API_CALL_MONITORING_KEY \
    "ezusb.api_call_monitoring"
#define IIDXHOOK_UTIL_CONFIG_EZUSB_IO_BOARD_TYPE_KEY "ezusb.io_board_type"

#define IIDXHOOK_UTIL_CONFIG_EZUSB_DEFAULT_API_CALL_MONITORING_VALUE false
#define IIDXHOOK_UTIL_CONFIG_EZUSB_DEFAULT_IO_BOARD_TYPE_VALUE 0

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
