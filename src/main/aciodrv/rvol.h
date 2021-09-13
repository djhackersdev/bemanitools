#ifndef ACIODRV_RVOL_H
#define ACIODRV_RVOL_H

#include "acio/rvol.h"

/**
 * Initialize a Revolte (KFCA 1.5) IO board
 *
 * @param device Context of opened device
 * @param node_id Id of the node to initialize (0 based).
 * @return True if successful, false on error.
 * @note This module is supposed to be used in combination with the common
 *       device driver foundation.
 * @see driver.h
 */
bool aciodrv_rvol_init(struct aciodrv_device_ctx *device, uint8_t node_id);

/**
 * Poll a Revolte (KFCA 1.5) IO board
 *
 * @param device Context of opened device
 * @param node_id Id of the node to query (0 based).
 * @param pout Pointer to a state struct to write to device
 * @param pin Pointer to a state struct to return the current state to
 * @return True on success, false on error.
 * @note This module is supposed to be used in combination with the common
 *       device driver foundation.
 * @see driver.h
 */
bool aciodrv_rvol_poll(
    struct aciodrv_device_ctx *device,
    uint8_t node_id,
    const struct ac_io_rvol_poll_out *pout,
    struct ac_io_rvol_poll_in *pin);

#endif
