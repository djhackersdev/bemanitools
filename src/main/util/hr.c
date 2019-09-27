#include <windows.h>

#include <stdint.h>

#include "util/hr.h"

HRESULT hr_from_win32(void)
{
    return HRESULT_FROM_WIN32(GetLastError());
}

void hr_propagate_win32_(HRESULT hr)
{
    uint32_t result;

    if (SUCCEEDED(hr)) {
        result = ERROR_SUCCESS;
    } else if (HRESULT_FACILITY(hr) == FACILITY_WIN32) {
        result = HRESULT_CODE(hr);
    } else {
        // https://docs.microsoft.com/en-us/windows/desktop/seccrypto/common-hresult-values
        switch (hr) {
        case E_ABORT:           result = ERROR_OPERATION_ABORTED; break;
        case E_ACCESSDENIED:    result = ERROR_ACCESS_DENIED; break;
        case E_FAIL:            result = ERROR_GEN_FAILURE; break;
        case E_HANDLE:          result = ERROR_INVALID_HANDLE; break;
        case E_INVALIDARG:      result = ERROR_INVALID_PARAMETER; break;
        case E_NOINTERFACE:     result = ERROR_NOT_SUPPORTED; break;
        case E_NOTIMPL:         result = ERROR_NOT_SUPPORTED; break;
        case E_OUTOFMEMORY:     result = ERROR_OUTOFMEMORY; break;
        case E_POINTER:         result = ERROR_INVALID_ADDRESS; break;
        case E_UNEXPECTED:      result = ERROR_INTERNAL_ERROR; break;
        default:                result = ERROR_INTERNAL_ERROR; break;
        }
    }

    SetLastError(result);
}

