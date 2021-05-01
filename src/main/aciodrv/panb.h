#ifndef ACIODRV_PANB_H
#define ACIODRV_PANB_H

#include "acio/panb.h"

/**
 * Send the AC_IO_CMD_PANB_START_AUTO_INPUT command on a PANB device.
 *
 * @param device Context of opened device.
 * @param node_id node_id Id of the node to query (0 based).
 * @param node_count The number of nodes to poll.
 * @return True if successful, false on error.
 * @note node_id should be 0, PANB internally manages the other nodes.
 * @note Upon calling, the device will stop replying to commands and just keep sending
 * AC_IO_CMD_PANB_POLL_REPLY messages indefinitely. Failure to retrieve them fast enough
 * will cause the device to malfunction. It is thus advised to make use of the aciodrv-proc
 * module to spawn a thread that will handle these messages and provide an easy access to the
 * latest input state.
 * @note This module is supposed to be used in combination with the common
 *       device driver foundation.
 * @see driver.h
 */
bool aciodrv_panb_start_auto_input(struct aciodrv_device_ctx *device, uint8_t node_id, uint8_t node_count);

/**
 * Retrieve a AC_IO_CMD_PANB_POLL_REPLY message from a PANB device. This assumes that there
 * is such message incoming (ie. that start_auto_input has been called prior).
 *
 * @param device Context of opened device.
 * @param poll_in Buffer to hold the received message, or NULL.
 * @return True if successful, false on error.
 * @note node_id should be 0, PANB internally manages the other nodes.
 * @note Failure to retrieve the incoming messages fast enough will cause the device to malfunction. 
 * It is thus advised to make use of the aciodrv-proc module to spawn a thread that will handle these 
 * messages and provide an easy access to the latest input state.
 * @note This module is supposed to be used in combination with the common
 *       device driver foundation.
 * @see driver.h
 */
bool aciodrv_panb_recv_poll(struct aciodrv_device_ctx *device, struct ac_io_panb_poll_in *poll_in);

/**
 * Light lamps on a PANB device.
 *
 * @param node_id Id of the node to query (0 based).
 * @param state Pointer to a lamp state struct 
 *        (mandatory).
 * @return True on success, false on error.
 * @note node_id should be 0, PANB internally manages the other nodes.
 * @note This module is supposed to be used in combination with the common
 *       device driver foundation.
 * @see driver.h
 */
bool aciodrv_panb_send_lamp(struct aciodrv_device_ctx *device, uint8_t node_id, struct ac_io_panb_poll_out *state);

#endif