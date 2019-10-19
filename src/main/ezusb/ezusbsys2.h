#ifndef EZUSB_EZUSBSYS2_H
#define EZUSB_EZUSBSYS2_H

#include <setupapi.h>
#include <usb100.h>
#include <windows.h>

#include "ezusbsys.h"

/* can't seem to #include the requisite DDK headers from usermode code,
   so we have to redefine these macros here */

#ifndef CTL_CODE
#define CTL_CODE(DeviceType, Function, Method, Access) \
    (((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))
#endif

#ifndef FILE_DEVICE_UNKNOWN
#define FILE_DEVICE_UNKNOWN 0x00000022
#endif

#ifndef METHOD_BUFFERED
#define METHOD_BUFFERED 0
#endif

#ifndef METHOD_IN_DIRECT
#define METHOD_IN_DIRECT 1
#endif

#ifndef METHOD_OUT_DIRECT
#define METHOD_OUT_DIRECT 2
#endif

#ifndef FILE_ANY_ACCESS
#define FILE_ANY_ACCESS 0
#endif

#endif