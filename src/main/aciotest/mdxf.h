#ifndef ACIOTEST_MDXF_H
#define ACIOTEST_MDXF_H

#include <stdbool.h>
#include <stdint.h>

#include "aciodrv/device.h"

bool aciotest_mdxf_handler_init(
    struct aciodrv_device_ctx *device, uint8_t node_id, void **ctx);
bool aciotest_mdxf_handler_update(
    struct aciodrv_device_ctx *device, uint8_t node_id, void *ctx);

#endif
