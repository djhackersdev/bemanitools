/**
 * Module for fundamental communication with a legacy Ezusb board, e.g.
 * IIDX C02 IO.
 * 
 * This module does not implement any sort of higher level protocol which is
 * required by the games to exchange data with the device, e.g. getting input
 * states of keys and setting light outputs. Further modules are utilizing this
 * base layer to implement higher level communication.
 * 
 * Note: This module requires the old "cyusb" driver which is NOT compatible
 * with anything newer than WinXP and does not support 64-bit systems. The newer
 * "cyusb3" driver does NOT work with this.
 * 
 * @author icex2 
 */
#ifndef EZUSB_H
#define EZUSB_H

#include <stdbool.h>
#include <stdint.h>

#include <windows.h>

#include "ezusb/util.h"

static const GUID EZUSB_GUID = {0xAE18AA60, 0x7F6A, 0x11D4,
    {0x97, 0xDD, 0x00, 0x01, 0x02, 0x29, 0xB9, 0x59}};

/* Used by IIDX instead of scanning for the device */
/* static const char* EZUSB_DEFAULT_PATHS[6] = {
    "\\\\.\\Ezusb-0",
    "\\\\.\\Ezusb-1",
    "\\\\.\\Ezusb-2",
    "\\\\.\\Ezusb-3",
    "\\\\.\\Ezusb-4",
    "\\\\.\\Ezusb-5"
}; */

#define EZUSB_DEVICE_PATH "\\\\.\\Ezusb-0"

/**
 * Open a connected Ezusb device. Using the old "ezusb" driver on WinXP,
 * the driver exposes the device to one of the following paths:
 * \\\\.\\Ezusb-0
 * \\\\.\\Ezusb-1
 * \\\\.\\Ezusb-2
 * \\\\.\\Ezusb-3
 * \\\\.\\Ezusb-4
 * \\\\.\\Ezusb-5
 * 
 * Where \\\\.\\Ezusb-X is used if slot X-1 is still occupied, e.g. when
 * flashing the device and after a usb re-enumerate.
 * 
 * @param device_path Path to the Ezusb device.
 * @return A valid handle to the device on success, INVALID_HANDLE_VALUE on 
 *         error.
 */
HANDLE ezusb_open(const char* path);

/**
 * Get identifier information of the device.
 * 
 * @param handle Valid device handle.
 * @param ident Pointer to memory to write the identifier information to.
 * @return True if successful and identifier information was written to the
 *         the struct, false on error.
 */
bool ezusb_get_ident(HANDLE handle, struct ezusb_ident* ident);

/**
 * Download a firmware to the ezusb device.
 * 
 * @param handle Valid device handle.
 * @param fw Firmware to write to the target ezusb device.
 * @return True on success, false on failure.
 */
bool ezusb_download_firmware(HANDLE handle, struct ezusb_firmware* fw);

/**
 * Close an opened ezusb device handle.
 * 
 * @param handle Valid handle to close.
 */
void ezusb_close(HANDLE handle);

#endif