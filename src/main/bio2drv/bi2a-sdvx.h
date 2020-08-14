#ifndef BIO2DRV_BI2A_H
#define BIO2DRV_BI2A_H

#include "bio2/bi2a-sdvx.h"

/**
 * Initialize a BI2A node.
 *
 * @param node_id Id of the node to initialize (0 based).
 * @return True if successful, false on error.
 * @note This module is supposed to be used in combination with the common
 *       device driver foundation.
 * @see driver.h
 */
bool bio2drv_bi2a_sdvx_init(uint8_t node_id);

/**
 * Poll the  board
 *
 * @param node_id Id of the node to query (0 based).
 * @param state Pointer to a state struct to return the current state to
 *        (optional, NULL for none).
 * @return True on success, false on error.
 * @note This module is supposed to be used in combination with the common
 *       device driver foundation.
 * @see driver.h
 */
bool bio2drv_bi2a_sdvx_poll(
    uint8_t node_id,
    const struct bi2a_sdvx_state_out *pout,
    struct bi2a_sdvx_state_in *pin);

/**
 * Set the BIO2 BI2A SDVX digital amp level
 *
 * @param node_id Id of the node to query (0 based).
 * @param primary primary volume (96-0)
 * @param headphone headphone volume (96-0)
 * @param unused unknown volume (96-0) (unused)
 * @param subwoofer subwoofer volume (96-0)
 * @return True on success, false on error.
 * @note Note 96 is lowest volume level, 0 is highest
 */
bool bio2drv_bi2a_sdvx_amp(
    uint8_t node_id,
    uint8_t unused_1,
    uint8_t unused_2,
    uint8_t left,
    uint8_t right);

#endif
