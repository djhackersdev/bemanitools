#define LOG_MODULE "p4ioemu-setupapi"

#include "p4ioemu/setupapi.h"

const struct hook_setupapi_data p4ioemu_setupapi_data = {
    .device_guid = {
        0x8B7250A5,
        0x4F61,
        0x46C9,
        { 0x84, 0x3A, 0xE6, 0x68, 0x06, 0x47, 0x6A, 0x20 }
    },
    .device_desc = NULL,
    .device_path = "\\p4io",
};