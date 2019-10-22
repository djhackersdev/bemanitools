#ifndef HOOKLIB_SETUPAPI_H
#define HOOKLIB_SETUPAPI_H

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <setupapi.h>
// clang-format on

struct hook_setupapi_data {
    GUID device_guid;
    const char *device_desc;
    const char *device_path;
};

void hook_setupapi_init(const struct hook_setupapi_data *data);

#endif
