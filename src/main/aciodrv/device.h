#ifndef ACIODRV_DEVICE_H
#define ACIODRV_DEVICE_H

#include <stdbool.h>

#include "acio/acio.h"

#include "windows.h"

#define ACIO_NODE_PRODUCT_CODE_LEN 4

struct aciodrv_device_ctx;

struct aciodrv_device_node_version {
    char product[ACIO_NODE_PRODUCT_CODE_LEN];
    uint32_t type;
    uint8_t major;
    uint8_t minor;
    uint8_t revision;
};

/**
 * Open an ACIO device connected to a serial port.
 *
 * @param port Port the device is connected to (e.g. "COM1")
 * @param baud Baud rate for communication (e.g. 57600 for ICCA)
 * @return opened device context, NULL on error
 */
struct aciodrv_device_ctx *aciodrv_device_open(const char *port_path, int baud)
#ifdef __GNUC__
    __attribute__((deprecated("Use aciomgr instead if device is shareable, "
                              "else aciodrv_device_open_path")))
#endif
    ;

struct aciodrv_device_ctx *
aciodrv_device_open_path(const char *port_path, int baud);

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
bool aciodrv_device_get_node_product_ident(
    struct aciodrv_device_ctx *device,
    uint8_t node_id,
    char product[ACIO_NODE_PRODUCT_CODE_LEN]);

/**
 * Get the product identifier of an enumerated node.
 *
 * @param device Context of opened device
 * @param node_id Id of the node. Needs to be in range of the total node count.
 * @return product type ID on success, or 0 on failure
 */
uint32_t aciodrv_device_get_node_product_type(
    struct aciodrv_device_ctx *device, uint8_t node_id);

/**
 * Get the product version of an enumerated node.
 *
 * @param device Context of opened device
 * @param node_id Id of the node. Needs to be in range of the total node count.
 * @return Pointer to the version struct
 */
const struct aciodrv_device_node_version *
aciodrv_device_get_node_product_version(
    struct aciodrv_device_ctx *device, uint8_t node_id);

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
bool aciodrv_send_and_recv(
    struct aciodrv_device_ctx *device,
    struct ac_io_message *msg,
    int max_resp_size);

/**
 * Send a message to the ACIO bus.
 *
 * @param device Context of opened device
 * @param msg Msg to send to the bus.
 * @return True on success, false on error.
 * @note Prefer the use of aciodrv_send_and_recv when possible. This is for
 * commands which don't trigger a reply.
 */
bool aciodrv_send(struct aciodrv_device_ctx *device, struct ac_io_message *msg);

/**
 * Read a message from the ACIO bus.
 *
 * @param device Context of opened device
 * @param msg Msg to send to the bus. Make sure that the buffer
 *        is big enough to receive the response.
 * @return True on success, false on error.
 * @note Prefer the use of aciodrv_send_and_recv when possible. This is for
 * unsollicited incoming messages.
 */
bool aciodrv_recv(
    struct aciodrv_device_ctx *device,
    struct ac_io_message *msg,
    int max_resp_size);

/**
 * Reset an opened device.
 *
 * @param device Context of opened device
 * @return Total num of nodes enumerated on the ACIO device.
 */
bool aciodrv_device_reset(struct aciodrv_device_ctx *device);

/**
 * Close the previously opened ACIO device.
 *
 * @param device Context of opened device
 */
void aciodrv_device_close(struct aciodrv_device_ctx *device);

#endif