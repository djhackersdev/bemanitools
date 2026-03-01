#ifndef ACIODRV_MDXF_H
#define ACIODRV_MDXF_H

#include "acio/mdxf.h"
#include "aciodrv/device.h"

bool aciodrv_mdxf_init(struct aciodrv_device_ctx *device, uint8_t node_id);

bool aciodrv_mdxf_poll(
    struct aciodrv_device_ctx *device,
    uint8_t node_id,
    const struct ac_io_mdxf_poll_in *pout);

#endif