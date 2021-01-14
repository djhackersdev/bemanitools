#ifndef ACIOTEST_HANDLER_H
#define ACIOTEST_HANDLER_H

#include "aciodrv/device.h"

static const uint8_t aciotest_handler_max = 16;

/**
 * Handler interface for an ACIO device.
 */
struct aciotest_handler_node_handler {
    void *ctx;
    bool (*init)(
        struct aciodrv_device_ctx *device, uint8_t node_id, void **ctx);
    bool (*update)(
        struct aciodrv_device_ctx *device, uint8_t node_id, void *ctx);
};

#endif