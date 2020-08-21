#include <windows.h>

#include <stdint.h>

#include "hook/hr.h"

uint32_t hr_to_win32_error(HRESULT hr)
{
    if (SUCCEEDED(hr)) {
        return ERROR_SUCCESS;
    } else if (HRESULT_FACILITY(hr) == FACILITY_WIN32) {
        return HRESULT_CODE(hr);
    } else {
        switch (hr) {
        case E_ABORT:           return ERROR_OPERATION_ABORTED;
        case E_ACCESSDENIED:    return ERROR_ACCESS_DENIED;
        case E_FAIL:            return ERROR_GEN_FAILURE;
        case E_HANDLE:          return ERROR_INVALID_HANDLE;
        case E_INVALIDARG:      return ERROR_INVALID_PARAMETER;
        case E_NOINTERFACE:     return ERROR_INVALID_FUNCTION;
        case E_NOTIMPL:         return ERROR_NOT_SUPPORTED;
        case E_OUTOFMEMORY:     return ERROR_OUTOFMEMORY;
        case E_POINTER:         return ERROR_INVALID_ADDRESS;
        case E_UNEXPECTED:      return ERROR_INTERNAL_ERROR;
        default:                return ERROR_INTERNAL_ERROR;
        }
    }
}

void hr_propagate_win32_(HRESULT hr)
{
    SetLastError(hr_to_win32_error(hr));
}
