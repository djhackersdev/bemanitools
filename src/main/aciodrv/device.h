#ifndef ACIODRV_DEVICE_H
#define ACIODRV_DEVICE_H

#include <stdbool.h>

#include "acio/acio.h"

/**
 * Open an ACIO device connected to a serial port.
 *
 * @param port Port the device is connected to (e.g. "COM1")
 * @param baud Baud rate for communication (e.g. 57600 for ICCA)
 * @return True if opening the port and resetting the device was successful,
 *         false on error.
 */
bool aciodrv_device_open(const char *port, int baud);

/**
 * Get the node count on the opened device.
 *
 * @return Total num of nodes enumerated on the ACIO device.
 */
uint8_t aciodrv_device_get_node_count(void);

/**
 * Get the product identifier of an enumerated node.
 *
 * @param node_id Id of the node. Needs to be in range of the total node count.
 * @param product Buffer to return the product id to.
 * @return True on success, false on error. If True the variable product
 * contains the identifier of the queried node.
 */
bool aciodrv_device_get_node_product_ident(uint8_t node_id, char product[4]);

/**
 * Send a message to the ACIO bus and receive an answer.
 * Use this to implement the protocol for each type of device that can be
 * part of the bus.
 *
 * @param msg Msg to send to the bus. Make sure that the buffer backing
 *        this message is big enough to receive the response as well.
 * @param resp_size Size of the expecting response.
 * @return True on success, false on error.
 */
bool aciodrv_send_and_recv(struct ac_io_message *msg, int resp_size);

/**
 * Close the previously opened ACIO device.
 */
void aciodrv_device_close(void);

#endif