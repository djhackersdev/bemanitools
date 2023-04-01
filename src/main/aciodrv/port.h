#ifndef ACIODRV_PORT_H
#define ACIODRV_PORT_H

#include <stdbool.h>
#include <stdint.h>

#include <windows.h>

/**
 * Open a serial port for communication with a ACIO device.
 *
 * @param port Port the device is connected to (e.g. "COM1")
 * @param baud Baud rate for communication (e.g. 57600 for ICCA)
 * @return HANDLE of the port, NULL on error
 * @note This will open and setup the com port, only.
 */
HANDLE aciodrv_port_open(const char *port_path, int baud);

/**
 * Read data from the opened com port.
 *
 * @param port_fd HANDLE of opened serial port
 * @param bytes Pointer to an allocated buffer to read the data into.
 * @param nbytes Number of bytes to read. Has to be less or equal the allocated
 *        buffer size.
 * @return Number of bytes read on success or -1 on error.
 */
int aciodrv_port_read(HANDLE port_fd, void *bytes, int nbytes);

/**
 * Write data to the opened com port.
 *
 * @param port_fd HANDLE of opened serial port
 * @param bytes Pointer to an allocated buffer with data to write.
 * @param nbytes Number of bytes to write. Has to be equal or less the size
 *        of the allocated buffer.
 * @return Number of bytes written on success or -1 on error.
 */
int aciodrv_port_write(HANDLE port_fd, const void *bytes, int nbytes);

/**
 * Close the previously opened com port.
 *
 * @param port_fd HANDLE of opened serial port
 */
void aciodrv_port_close(HANDLE port_fd);

#endif