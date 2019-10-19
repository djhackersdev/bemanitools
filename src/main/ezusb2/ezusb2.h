/**
 * Module for fundamental communication with an Ezusb FX2 board, e.g.
 * IIDX/Pop'n IO2. Credit also goes to willxinc for his initial implemention
 * which, for some unknown reason, did not work on my machines.
 *
 * This module does not implement any sort of higher level protocol which is
 * required by the games to exchange data with the device, e.g. getting input
 * states of keys and setting light outputs. Further modules are utilizing this
 * base layer to implement higher level communication.
 *
 * Note: This module requires the newer "cyusb3" driver which is compatible
 * with WinXP, Win7, Win8 and Win10 (both 32 and 64-bit). The old "cyusb" driver
 * does NOT work with this.
 *
 * @author icex2, willxinc
 */
#ifndef EZUSB2_H
#define EZUSB2_H

#include <stdbool.h>
#include <stdint.h>

#include <windows.h>

#include "ezusb/util.h"

/**
 * GUID of Ezusb FX2 board
 */
static const GUID EZUSB2_GUID = {
    0xAE18AA60L,
    0x7F6A,
    0x11D4,
    {0x97, 0xDD, 0x00, 0x01, 0x02, 0x29, 0xB9, 0x59}};

/**
 * Scan for connected devices which match the provided GUID.
 *
 * @param guid GUID of device to find.
 * @return Allocated device path string of first device found matching the GUID
 *         provided. Caller has to free buffer.
 */
char *ezusb2_find(const GUID *guid);

/**
 * Open a connected Ezusb FX2 device.
 *
 * @param device_path Path to the Ezusb FX2 device obtained by using
 *        the ezusb2_find function.
 * @return A valid handle to the device on success, INVALID_HANDLE_VALUE on
 *         error.
 */
HANDLE ezusb2_open(const char *device_path);

/**
 * Get identifier information of the device.
 *
 * @param handle Valid device handle.
 * @param ident Pointer to memory to write the identifier information to.
 * @return True if successful and identifier information was written to the
 *         the struct, false on error.
 */
bool ezusb2_get_ident(HANDLE handle, struct ezusb_ident *ident);

/**
 * Download a firmware to the ezusb device.
 *
 * @param handle Valid device handle.
 * @param fw Firmware to write to the target ezusb device.
 * @return True on success, false on failure.
 */
bool ezusb2_download_firmware(HANDLE handle, struct ezusb_firmware *fw);

/**
 * Execute a data transfer to a specific endpoint other than endpoint 0.
 *
 * @param handle Valid device handle.
 * @param endpoint Target endpoint, in and out valid but not 0.
 * @param data Buffer with data to transfer or for data to receive.
 * @param size Size of buffer.
 * @return True on success, false on failure.
 */
bool ezusb2_endpoint_transfer(
    HANDLE handle, uint8_t endpoint, void *data, uint32_t size);

/**
 * Close an opened ezusb device handle.
 *
 * @param handle Valid handle to close.
 */
void ezusb2_close(HANDLE handle);

#endif