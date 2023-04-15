#ifndef IIDXHOOK_D3D9_UTIL_H
#define IIDXHOOK_D3D9_UTIL_H

#include <windows.h>

#include "d3d9-util/dxerr9.h"

#include "util/log.h"

inline void
iidxhook_d3d9_util_check_and_handle_failure(HRESULT hr, const char *msg)
{
    if (FAILED(hr)) {
        log_fatal(
            "%s\nDX error: %s, DX error description: %s",
            msg,
            DXGetErrorString9(hr),
            DXGetErrorDescription9(hr));
    }
}

#endif