#ifndef IIDXHOOK_UTIL_CONFIG_EZUSB_H
#define IIDXHOOK_UTIL_CONFIG_EZUSB_H

#include "cconfig/cconfig.h"

#include "iface-core/config.h"

typedef struct iidxhook_util_config_ezusb {
    bool api_call_monitoring;
    int32_t io_board_type;
} iidxhook_util_config_ezusb_t;

void iidxhook_util_config_ezusb_init(struct cconfig *config);

void iidxhook_util_config_ezusb_get(
    struct iidxhook_util_config_ezusb *config_ezusb, struct cconfig *config);

void iidxhook_util_config_ezusb_get2(
    const bt_core_config_t *config,
    iidxhook_util_config_ezusb_t *config_ezusb);

#endif