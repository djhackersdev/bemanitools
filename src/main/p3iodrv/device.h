#ifndef P3IODRV_DEVICE_H
#define P3IODRV_DEVICE_H

#include <stdbool.h>
#include <stddef.h>
#include <windows.h>

#include "p3io/cmd.h"

#include "util/iobuf.h"

#define P3IODRV_VERSION_MAX_LEN 128
#define P3IO_DRV_JAMMA_MAX_LEN 3

/**
 * Scan for a connected p3io device. Currently, this does not support multiple
 * p3io devices and only returns the "first one" found.
 *
 * @param path Buffer of size MAX_PATH this function writes the full device path
 *        to when a device was found
 * @return S_OK when a device was found, ERROR_FILE_NOT_FOUND if none was found
 *         or any other HRESULT error on any errors while scanning occured.
 */
HRESULT p3iodrv_device_scan(char path[MAX_PATH]);

/**
 * Open a p3io device by the given device path.
 *
 * @param path Pointer to a buffer with the full device path of the p3io device
 *        to open
 * @param handle Pointer to a handle variable to return the opened device handle
 *        to when opening was successful
 * @return S_OK if opening was successful, ERROR_FILE_NOT_FOUND if the device
 *         with the given path was not found, or any other HRESULT error when
 *         trying to open the device.
 */
HRESULT p3iodrv_device_open(const char *path, HANDLE *handle);

/**
 * Close an open p3io device by its handle
 *
 * @param handle Pointer to the handle to close. The variable will be set to
 *        INVALID_HANDLE_VALUE on success
 * @return S_OK if closing was successful, or any other HRESULT value on errors
 */
HRESULT p3iodrv_device_close(HANDLE *handle);

/**
 * Read the version of the p3io device.
 *
 * This call is not guaranteed to be supported with all p3io drivers, e.g. we
 * know this is supported by the 64-bit p3io driver but apparently not by the
 * 32-bit one.
 *
 * @param handle A valid handle to an opened p3io device
 * @param version Pointer to a buffer to write the resulting version string to
 * @return S_OK if the operation was successful, or any other HRESULT value on
 *         errors
 */
HRESULT p3iodrv_device_read_version(
    HANDLE handle, char version[P3IODRV_VERSION_MAX_LEN]);

/**
 * Read input data from the JAMMA edge. As this is supported by different games
 * and a core functionality of the p3io, the data is not further specified
 * here. This function is optimised for low latency inputs by continously
 * polling it on the host.
 *
 * @param handle A valid handle to an opened p3io device
 * @param jamma Buffer to write the read input data to on success
 * @return S_OK if the operation was successful, or any other HRESULT value on
 *         errors
 */
HRESULT p3iodrv_device_read_jamma(
    HANDLE handle, uint32_t jamma[P3IO_DRV_JAMMA_MAX_LEN]);

/**
 * Execute a generic data transfer.
 *
 * This is _the_ generic messaging interface of the p3io to exchange data with
 * the device. Depending on the type of p3io and firmware, different command
 * codes support different functionality.
 *
 * Note that this interface is not optimized for low latency data transfers
 * and is a fully blocking "endpoint". The caller of this function is blocked
 * entirely until the host request is fully processed and the device response is
 * fully received.
 *
 * Implementation detail: It implements (sort of?) the ACIO protocol on top of
 * the USB as a transport layer. Encoding and framing is being handling by this
 * function transparently, so the caller does not have to worry about that
 * anymore.
 *
 * @param handle A valid handle to an opened p3io device
 * @param req Pointer to a buffer/struct with a p3io formatted request
 * @param resp Pointer to a buffer to receive the device's response to. Ensure
 *             this is large enough for the expceted response matching the
 *             issued type of request.
 * @return S_OK if the transfer was successful and a response was received and
 *         written to the buffer, or any other HRESULT value on errors
 */
HRESULT p3iodrv_device_transfer(
    HANDLE handle, const union p3io_req_any *req, union p3io_resp_any *resp);

#endif