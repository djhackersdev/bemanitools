#ifndef ACIODRV_ICCA_H
#define ACIODRV_ICCA_H

#include "acio/icca.h"

/**
 * Initialize an ICCA node.
 *
 * @param device Context of opened device
 * @param node_id Id of the node to initialize (0 based).
 * @return True if successful, false on error.
 * @note This module is supposed to be used in combination with the common
 *       device driver foundation.
 * @see driver.h
 */
bool aciodrv_icca_init(struct aciodrv_device_ctx *device, uint8_t node_id);

/**
 * Set the state of on ICCA node.
 *
 * @param device Context of opened device
 * @param node_id Id of the node to set the state for (0 based).
 * @param slot_state State of the slot (refer to corresponding enum).
 * @param state Pointer to a state struct to return the current state to
 *        (optional, NULL for none).
 * @return True on success, false on error.
 * @note This module is supposed to be used in combination with the common
 *       device driver foundation.
 * @see driver.h
 */
bool aciodrv_icca_set_state(
    struct aciodrv_device_ctx *device,
    uint8_t node_id,
    int slot_state,
    struct ac_io_icca_state *state);

/**
 * Get the current state of an ICCA node.
 *
 * @param device Context of opened device
 * @param node_id Id of the node to query (0 based).
 * @param state Pointer to a state struct to return the current state to
 *        (optional, NULL for none).
 * @return True on success, false on error.
 * @note This module is supposed to be used in combination with the common
 *       device driver foundation.
 * @see driver.h
 */
bool aciodrv_icca_get_state(
    struct aciodrv_device_ctx *device,
    uint8_t node_id,
    struct ac_io_icca_state *state);

/**
 * Trigger a card read action on the ICCA reader. Make sure to call this
 * when you want to read the card. Just polling the state is not sufficient
 * to get the most recent card data. Make sure to re-get the state after
 * a read call. The state returned here might not be up to date for some reason.
 *
 * @param device Context of opened device
 * @param node_id Id of the node to query (0 based).
 * @param state Pointer to a state struct to return the current state to
 *        (optional, NULL for none).
 * @return True on success, false on error.
 * @note This module is supposed to be used in combination with the common
 *       device driver foundation.
 * @see driver.h
 */
bool aciodrv_icca_read_card(
    struct aciodrv_device_ctx *device,
    uint8_t node_id,
    struct ac_io_icca_state *state);

/**
 * Uses some heruistics based on the product ident to determine if the detected
 * device is a slotted (true) or wavepass reader (false).
 * 
 * This function will also return false if the provided node_id is invalid.
 * 
 * @param device Context of opened device
 * @param node_id Id of the node to query (0 based).
 * @return True on slotted, false on wavepass.
 * @note This module is supposed to be used in combination with the common
 *       device driver foundation.
 * @see driver.h
 */
bool aciodrv_icca_is_slotted(
    struct aciodrv_device_ctx *device,
    uint8_t node_id);

/**
 * Polls the felica chip on wavepass readers. This will cause the state of the
 * reader to be AC_IO_ICCA_STATUS_BUSY_NEW for a few polls, before returning
 * either: AC_IO_ICCA_STATUS_IDLE_NEW or AC_IO_ICCA_STATUS_GOT_UID.
 * 
 * The user should take care to call this every so often to actually get new
 * felica UIDs, instead of just old NFC idents. The game seems to do it every
 * 5 or so polls after the last AC_IO_ICCA_STATUS_BUSY_NEW poll.
 * 
 * @param device Context of opened device
 * @param node_id Id of the node to query (0 based).
 * @return True on success, false on error.
 * @note This module is supposed to be used in combination with the common
 *       device driver foundation.
 * @see driver.h
 */
bool aciodrv_icca_poll_felica(
    struct aciodrv_device_ctx *device,
    uint8_t node_id);

#endif
