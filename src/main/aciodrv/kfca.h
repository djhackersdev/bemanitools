#ifndef ACIODRV_KFCA_H
#define ACIODRV_KFCA_H

#include "acio/kfca.h"

/**
 * Initialize an KFCA node.
 *
 * @param node_id Id of the node to initialize (0 based).
 * @return True if successful, false on error.
 * @note This module is supposed to be used in combination with the common
 *       device driver foundation.
 * @see driver.h
 */
bool aciodrv_kfca_init(uint8_t node_id);

/**
 * Poll the KFCA io board
 *
 * @param node_id Id of the node to query (0 based).
 * @param state Pointer to a state struct to return the current state to
 *        (optional, NULL for none).
 * @return True on success, false on error.
 * @note This module is supposed to be used in combination with the common
 *       device driver foundation.
 * @see driver.h
 */
bool aciodrv_kfca_poll(uint8_t node_id, const struct ac_io_kfca_poll_out* pout, struct ac_io_kfca_poll_in* pin);

#endif
