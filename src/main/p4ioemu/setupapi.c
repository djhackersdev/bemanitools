#define LOG_MODULE "p4ioemu-setupapi"

#include "p4ioemu/setupapi.h"

#include "p4io/guid.h"

const struct hook_setupapi_data p4ioemu_setupapi_data = {
    .device_guid = p4io_guid,
    .device_desc = NULL,
    .device_path = "\\p4io",
};
