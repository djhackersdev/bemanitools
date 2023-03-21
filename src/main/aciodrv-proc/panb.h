#ifndef ACIODRV_PROC_PANB_H
#define ACIODRV_PROC_PANB_H

#include "aciodrv/panb.h"

/**
 * Initialize a PANB device. This will take care of setting up the auto-poll and
 * processing the poll messages in a separate thread (this is necessary as
 * failing to process messages fast enough will cause the device to
 * malfunction).
 *
 * @param device Context of opened device
 * @return True if successful, false on error.
 * @note This function spawns a thread. Caller must call aciodrv_proc_panb_fini
 * to properly terminate.
 */
bool aciodrv_proc_panb_init(struct aciodrv_device_ctx *device);

/**
 * Retrieve latest known button state from the PANB device.
 *
 * @param button_state 28 cell array to store button state
 *        (mandatory, upon calling the array contains values between 0 to 15
 * indicating the keys velocity).
 * @return True on success, false on error.
 */
bool aciodrv_proc_panb_get_state(uint8_t *button_state);

/**
 * Properly terminate the thread and reset the device (this is the only way to
 * stop the auto polling).
 */
void aciodrv_proc_panb_fini(struct aciodrv_device_ctx *device);

#endif