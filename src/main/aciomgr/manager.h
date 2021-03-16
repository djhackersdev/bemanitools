#ifndef ACIOMGR_MANAGER_H
#define ACIOMGR_MANAGER_H

#include <stdbool.h>

#include "acio/acio.h"

#include "bemanitools/glue.h"

#define ACIOMGR_NODE_PRODUCT_CODE_LEN 4

struct aciodrv_device_ctx;
struct aciomgr_port_dispatcher;

// internal
void _aciomgr_init();
void _aciomgr_fini();

/**
 * The first function that will be called on your DLL. You will be supplied
 * with four function pointers that may be used to log messages to the game's
 * log file. See comments in glue.h for further information.
 */
void aciomgr_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal);

/**
 * Init and open, or return the existing handle to an acio port
 *
 * @param path Port the device is connected to (e.g. "COM1")
 * @return opened port dispatcher context, NULL on error
 */
struct aciomgr_port_dispatcher *aciomgr_port_init(const char *path, int baud);

void aciomgr_port_fini(struct aciomgr_port_dispatcher *dispatcher);

/**
 * Get the node count on the opened device.
 *
 * @param dispatcher dispatcher context from aciomgr_port_init
 * @return Total num of nodes enumerated on the ACIO device.
 */
uint8_t aciomgr_get_node_count(struct aciomgr_port_dispatcher *dispatcher);

/**
 * Get the product identifier of an enumerated node.
 *
 * @param dispatcher dispatcher context from aciomgr_port_init
 * @param node_id Id of the node. Needs to be in range of the total node count.
 * @param product Buffer to return the product id to.
 * @return True on success, false on error. If True the variable product
 * contains the identifier of the queried node.
 */
bool aciomgr_get_node_product_ident(
    struct aciomgr_port_dispatcher *dispatcher,
    uint8_t node_id,
    char product[ACIOMGR_NODE_PRODUCT_CODE_LEN]);

/**
 * Submit and wait for the response of the specified packet
 *
 * @param dispatcher dispatcher context from aciomgr_port_init
 * @param msg pointer to the message buffer to read from / write to
 * @param max_resp_size pointer to the max size response we expect
 * @return false on error
 */
bool aciomgr_port_submit_packet(
    struct aciomgr_port_dispatcher *dispatcher,
    struct ac_io_message *msg,
    int max_resp_size);

/**
 * Checkout the device handler for this port dispatcher on this thread
 *
 * @param dispatcher dispatcher context from aciomgr_port_init
 * @return the device context
 */
struct aciodrv_device_ctx *
aciomgr_port_checkout(struct aciomgr_port_dispatcher *dispatcher);

/**
 * Checkin the device handler that this thread holds
 *
 * @param dispatcher dispatcher context from aciomgr_port_init
 */
void aciomgr_port_checkin(struct aciomgr_port_dispatcher *dispatcher);

#endif