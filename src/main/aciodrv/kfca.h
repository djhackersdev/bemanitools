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
bool aciodrv_kfca_poll(
    uint8_t node_id,
    const struct ac_io_kfca_poll_out *pout,
    struct ac_io_kfca_poll_in *pin);

/**
 * Set the KFCA digital amp level
 *
 * @param node_id Id of the node to query (0 based).
 * @param primary primary volume (96-0)
 * @param headphone headphone volume (96-0)
 * @param unused unknown volume (96-0) (unused)
 * @param subwoofer subwoofer volume (96-0)
 * @return True on success, false on error.
 * @note Note 96 (or 100?) is lowest volume level, 0 is highest
 */
bool aciodrv_kfca_amp(
    uint8_t node_id,
    uint8_t primary,
    uint8_t headphone,
    uint8_t unused,
    uint8_t subwoofer);

#endif
