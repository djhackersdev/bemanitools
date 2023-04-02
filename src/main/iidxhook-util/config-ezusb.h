#ifndef IIDXHOOK_UTIL_CONFIG_EZUSB_H
#define IIDXHOOK_UTIL_CONFIG_EZUSB_H

#include "cconfig/cconfig.h"

#include "security/mcode.h"

struct iidxhook_util_config_ezusb {
    bool api_call_monitoring;
    int32_t io_board_type;
};

void iidxhook_util_config_ezusb_init(struct cconfig *config);

void iidxhook_util_config_ezusb_get(
    struct iidxhook_util_config_ezusb *config_ezusb, struct cconfig *config);

#endif