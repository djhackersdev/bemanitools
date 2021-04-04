#ifndef P4IO_IOCTL_H
#define P4IO_IOCTL_H

/* can't seem to #include the requisite DDK headers from usermode code,
   so we have to redefine these macros here */

#ifndef CTL_CODE
#define CTL_CODE(DeviceType, Function, Method, Access) \
    (((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))
#endif

#ifndef METHOD_BUFFERED
#define METHOD_BUFFERED 0
#endif

#ifndef FILE_ANY_ACCESS
#define FILE_ANY_ACCESS 0x00
#endif

#ifndef FILE_DEVICE_UNKNOWN
#define FILE_DEVICE_UNKNOWN 0x22
#endif

#define P4IO_FUNCTION_READ_JAMMA_2 0x801
#define P4IO_FUNCTION_GET_DEVICE_NAME 0x803

// 0x22200Cu sent to bulk handle
#define P4IO_IOCTL_GET_DEVICE_NAME     \
    CTL_CODE(                          \
        FILE_DEVICE_UNKNOWN,           \
        P4IO_FUNCTION_GET_DEVICE_NAME, \
        METHOD_BUFFERED,               \
        FILE_ANY_ACCESS)
// 0x222004 sent to int handle
#define P4IO_IOCTL_READ_JAMMA_2     \
    CTL_CODE(                       \
        FILE_DEVICE_UNKNOWN,        \
        P4IO_FUNCTION_READ_JAMMA_2, \
        METHOD_BUFFERED,            \
        FILE_ANY_ACCESS)

#endif
