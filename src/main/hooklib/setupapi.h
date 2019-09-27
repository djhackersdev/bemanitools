#ifndef HOOKLIB_SETUPAPI_H
#define HOOKLIB_SETUPAPI_H

#include <windows.h>
#include <setupapi.h>

struct hook_setupapi_data {
    GUID device_guid;
    const char* device_desc;
    const char* device_path;
};

void hook_setupapi_init(const struct hook_setupapi_data* data);

#endif
