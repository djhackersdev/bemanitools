#ifndef ACIOTEST_BI2A_IIDX_H
#define ACIOTEST_BI2A_IIDX_H

#include <stdbool.h>
#include <stdint.h>

#include "aciodrv/device.h"

bool aciotest_bi2a_iidx_handler_init(
    struct aciodrv_device_ctx *device, uint8_t node_id, void **ctx);
bool aciotest_bi2a_iidx_handler_update(
    struct aciodrv_device_ctx *device, uint8_t node_id, void *ctx);

#endif
