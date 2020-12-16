#ifndef BIO2DRV_BI2A_IIDX_H
#define BIO2DRV_BI2A_IIDX_H

#include "bio2/bi2a-iidx.h"

/**
 * Initialize a BI2A node.
 *
 * @param node_id Id of the node to initialize (0 based).
 * @return True if successful, false on error.
 * @note This module is supposed to be used in combination with the common
 *       device driver foundation.
 * @see driver.h
 */
bool bio2drv_bi2a_iidx_init(uint8_t node_id);

/**
 * Poll the BI2A board
 *
 * @param node_id Id of the node to query (0 based).
 * @param state Pointer to a state struct to return the current state to
 *        (optional, NULL for none).
 * @return True on success, false on error.
 * @note This module is supposed to be used in combination with the common
 *       device driver foundation.
 * @see driver.h
 */
bool bio2drv_bi2a_iidx_poll(
    uint8_t node_id,
    const struct bi2a_iidx_state_out *pout,
    struct bi2a_iidx_state_in *pin);

#endif
