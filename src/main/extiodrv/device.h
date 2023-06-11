#ifndef EXTIODRV_DEVICE_H
#define EXTIODRV_DEVICE_H

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

/**
 * Open an EXTIO device connected to a com port.
 *
 * @param port Com port the device is connected to, e.g. "COM1"
 * @param handle Pointer to a HANDLE variable to return the handle to when
 *               succesfully opened
 * @return S_OK on success with the handle returned in handle, other HRESULT
 *         value on error.
 */
HRESULT extiodrv_device_open(const char *port, HANDLE *handle);

/**
 * Close an opened EXTIO device
 *
 * @param handle Pointer to a handle variable that stores a valid and opened
 *        EXTIO handle.
 * @return S_OK on success and the handle value is set to INVALID_HANDLE_VALUE,
 *         other HRESULT value on error.
 */
HRESULT extiodrv_device_close(HANDLE *handle);

/**
 * Read data from the EXTIO device. This call blocks until data is available.
 *
 * @param handle A valid handle to an opened EXTIO device.
 * @param bytes Pointer to a buffer to read the data into
 * @param nbytes (Maximum) number of bytes to read
 * @param read_bytes Pointer to a variable to store the resulting read size to
 * @return S_OK on success with read_bytes populated with the number of bytes
 *         read, other HRESULT value on error.
 */
HRESULT extiodrv_device_read(
    HANDLE handle, void *bytes, size_t nbytes, size_t *read_bytes);

/**
 * WRite data to the EXTIO device. This call blocks until the data is written.
 *
 * @param handle A valid handle to an opened EXTIO device.
 * @param bytes Pointer to a buffer with data to write to the device
 * @param nbytes (Maximum) number of bytes to write
 * @param written_bytes Pointer to a variable to store the resulting write size
 *        to
 * @return S_OK on success with written_bytes populated with the number of bytes
 *         written, other HRESULT value on error.
 */
HRESULT extiodrv_device_write(
    HANDLE handle, const void *bytes, size_t nbytes, size_t *written_bytes);

#endif