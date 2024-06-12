#ifndef BT_ACIO_MGR_H
#define BT_ACIO_MGR_H

#include <stdbool.h>
#include <stdint.h>

#include "api/acio/mgr.h"

void bt_acio_mgr_api_set(const bt_acio_mgr_api_t *api);
void bt_acio_mgr_api_get(bt_acio_mgr_api_t *api);
void bt_acio_mgr_api_clear();

/**
 * Init and open, or return the existing handle to an acio port
 *
 * @param path Port the device is connected to (e.g. "COM1")
 * @return opened port dispatcher context, NULL on error
 */
bt_acio_mgr_port_dispatcher_t *
bt_acio_mgr_port_init(const char *path, uint32_t baud);

void bt_acio_mgr_port_fini(bt_acio_mgr_port_dispatcher_t *dispatcher);

/**
 * Get the node count on the opened device.
 *
 * @param dispatcher dispatcher context from aciomgr_port_init
 * @return Total num of nodes enumerated on the ACIO device.
 */
uint8_t
bt_acio_mgr_node_count_get(const bt_acio_mgr_port_dispatcher_t *dispatcher);

/**
 * Get the product identifier of an enumerated node.
 *
 * @param dispatcher dispatcher context from aciomgr_port_init
 * @param node_id Id of the node. Needs to be in range of the total node count.
 * @param product Buffer to return the product id to.
 * @return True on success, false on error. If True the variable product
 * contains the identifier of the queried node.
 */
bool bt_acio_mgr_node_product_ident_get(
    const bt_acio_mgr_port_dispatcher_t *dispatcher,
    uint8_t node_id,
    char product[BT_ACIO_MGR_NODE_PRODUCT_CODE_LEN]);

/**
 * Submit and wait for the response of the specified packet
 *
 * @param dispatcher dispatcher context from aciomgr_port_init
 * @param msg pointer to the message buffer to read from / write to
 * @param max_resp_size pointer to the max size response we expect
 * @return false on error
 */
bool bt_acio_mgr_port_packet_submit(
    const bt_acio_mgr_port_dispatcher_t *dispatcher,
    const bt_acio_message_t *msg,
    uint32_t max_resp_size);

/**
 * Checkout the device handler for this port dispatcher on this thread
 *
 * @param dispatcher dispatcher context from aciomgr_port_init
 * @return the device context
 */
bt_acio_drv_device_ctx_t *
bt_acio_mgr_port_checkout(const bt_acio_mgr_port_dispatcher_t *dispatcher);

/**
 * Checkin the device handler that this thread holds
 *
 * @param dispatcher dispatcher context from aciomgr_port_init
 */
void bt_acio_mgr_port_checkin(const bt_acio_mgr_port_dispatcher_t *dispatcher);

#endif