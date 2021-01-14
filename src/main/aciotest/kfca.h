#ifndef ACIOTEST_KFCA_H
#define ACIOTEST_KFCA_H

#include <stdbool.h>
#include <stdint.h>

#include "aciodrv/device.h"

bool aciotest_kfca_handler_init(
    struct aciodrv_device_ctx *device, uint8_t node_id, void **ctx);
bool aciotest_kfca_handler_update(
    struct aciodrv_device_ctx *device, uint8_t node_id, void *ctx);

#endif
