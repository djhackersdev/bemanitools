/**
 * This is the header for ch341dll.dll, a library for interfacing with the
 * CH341 USB to serial/parallel port chip. Magic Box uses this for their
 * proprietary IO, and it provides access to the panel sensors.
 *
 * The full .h for this file is floating around online with many more functions
 * and comments (in Chinese). Only the bare minimum definitions are provided
 * here for use in jbio-magicbox
 */

#ifndef IMPORTS_CH341_H
#define IMPORTS_CH341_H

HANDLE WINAPI CH341OpenDevice(ULONG iIndex);
VOID WINAPI CH341CloseDevice(ULONG iIndex);
BOOL WINAPI CH341EppReadData(ULONG iIndex, PVOID oBuffer, PULONG ioLength);
BOOL WINAPI CH341EppSetAddr(ULONG iIndex, UCHAR iAddr);

#endif
