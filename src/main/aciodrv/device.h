#ifndef ACIODRV_DEVICE_H
#define ACIODRV_DEVICE_H

#include <stdbool.h>

#include "acio/acio.h"

#include "windows.h"

#define ACIO_NODE_PRODUCT_CODE_LEN 4

struct aciodrv_device_ctx;

/**
 * Open an ACIO device connected to a serial port.
 *
 * @param port Port the device is connected to (e.g. "COM1")
 * @param baud Baud rate for communication (e.g. 57600 for ICCA)
 * @return opened device context, NULL on error
 */
struct aciodrv_device_ctx *aciodrv_device_open(const char *port_path, int baud);

/**
 * Get the node count on the opened device.
 *
 * @param device Context of opened device
 * @return Total num of nodes enumerated on the ACIO device.
 */
uint8_t aciodrv_device_get_node_count(struct aciodrv_device_ctx *device);

/**
 * Get the product identifier of an enumerated node.
 *
 * @param device Context of opened device
 * @param node_id Id of the node. Needs to be in range of the total node count.
 * @param product Buffer to return the product id to.
 * @return True on success, false on error. If True the variable product
 * contains the identifier of the queried node.
 */
bool aciodrv_device_get_node_product_ident(struct aciodrv_device_ctx *device, uint8_t node_id, char product[ACIO_NODE_PRODUCT_CODE_LEN]);

/**
 * Send a message to the ACIO bus and receive an answer.
 * Use this to implement the protocol for each type of device that can be
 * part of the bus.
 *
 * @param device Context of opened device
 * @param msg Msg to send to the bus. Make sure that the buffer backing
 *        this message is big enough to receive the response as well.
 * @param resp_size Size of the expecting response.
 * @return True on success, false on error.
 */
bool aciodrv_send_and_recv(struct aciodrv_device_ctx *device, struct ac_io_message *msg, int max_resp_size);

/**
 * Close the previously opened ACIO device.
 * 
 * @param device Context of opened device
 */
void aciodrv_device_close(struct aciodrv_device_ctx *device);

#endif