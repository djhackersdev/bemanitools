#pragma once

#include <windows.h>

#include <stdint.h>

typedef int (*usbCheckAlive_t)();
typedef int (*usbCheckSecurityNew_t)();
typedef int (*usbCoinGet_t)();
typedef int (*usbCoinMode_t)();
typedef int (*usbEnd_t)();
typedef int (*usbFirmResult_t)();
typedef int (*usbGetKEYID_t)();
typedef int (*usbGetSecurity_t)();
typedef int (*usbLamp_t)(uint32_t lamp_bits);
typedef int (*usbPadRead_t)(unsigned int *pad_bits);
typedef int (*usbPadReadLast_t)(uint8_t *a1);
typedef int (*usbSecurityInit_t)();
typedef int (*usbSecurityInitDone_t)();
typedef int (*usbSecuritySearch_t)();
typedef int (*usbSecuritySearchDone_t)();
typedef int (*usbSecuritySelect_t)();
typedef int (*usbSecuritySelectDone_t)();
typedef int (*usbSetExtIo_t)();
typedef int (*usbStart_t)();
typedef int (*usbWdtReset_t)();
typedef int (*usbWdtStart_t)(int a1);
typedef int (*usbWdtStartDone_t)();

typedef BOOL __stdcall (*DllEntryPoint_t)(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);