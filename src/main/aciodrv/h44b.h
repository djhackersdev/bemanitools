#ifndef ACIODRV_H44B_H
#define ACIODRV_H44B_H

#include "aciodrv/device.h"
#include "acio/h44b.h"

/**
 * Initialize an H44B node.
 *
 * @param device Context of opened device
 * @param node_id Id of the node to initialize (0 based).
 * @return True if successful, false on error.
 * @note This module is supposed to be used in combination with the common
 *       device driver foundation.
 * @see driver.h
 */
bool aciodrv_h44b_init(struct aciodrv_device_ctx *device, uint8_t node_id);

/**
 * Set the H44B LEDs
 *
 * @param device Context of opened device
 * @param node_id Id of the node to query (0 based).
 * @param lights Packed lights struct to send
 * @return True on success, false on error.
 */
bool aciodrv_h44b_lights(
    struct aciodrv_device_ctx *device,
    uint8_t node_id,
    const struct ac_io_h44b_output *lights);

#endif
