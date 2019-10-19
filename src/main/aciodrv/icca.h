#ifndef ACIODRV_ICCA_H
#define ACIODRV_ICCA_H

#include "acio/icca.h"

/**
 * Initialize an ICCA node.
 *
 * @param node_id Id of the node to initialize (0 based).
 * @return True if successful, false on error.
 * @note This module is supposed to be used in combination with the common
 *       device driver foundation.
 * @see driver.h
 */
bool aciodrv_icca_init(uint8_t node_id);

/**
 * Set the state of on ICCA node.
 *
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
    uint8_t node_id, int slot_state, struct ac_io_icca_state *state);

/**
 * Get the current state of an ICCA node.
 *
 * @param node_id Id of the node to query (0 based).
 * @param state Pointer to a state struct to return the current state to
 *        (optional, NULL for none).
 * @return True on success, false on error.
 * @note This module is supposed to be used in combination with the common
 *       device driver foundation.
 * @see driver.h
 */
bool aciodrv_icca_get_state(uint8_t node_id, struct ac_io_icca_state *state);

/**
 * Trigger a card read action on the ICCA reader. Make sure to call this
 * when you want to read the card. Just polling the state is not sufficient
 * to get the most recent card data. Make sure to re-get the state after
 * a read call. The state returned here might not be up to date for some reason.
 *
 * @param node_id Id of the node to query (0 based).
 * @param state Pointer to a state struct to return the current state to
 *        (optional, NULL for none).
 * @return True on success, false on error.
 * @note This module is supposed to be used in combination with the common
 *       device driver foundation.
 * @see driver.h
 */
bool aciodrv_icca_read_card(uint8_t node_id, struct ac_io_icca_state *state);

#endif