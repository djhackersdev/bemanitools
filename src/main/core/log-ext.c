

#include <windows.h>

#include "iface-core/log.h"

void core_log_ext_win_last_error_log(const char *module, bt_core_log_message_t log_message)
{
    LPSTR buffer;
    DWORD last_error;

    last_error = GetLastError();

    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        last_error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR) &buffer,
        0,
        NULL);

    log_message(module, "Last error (%08X): %s", last_error, buffer);

    LocalFree(buffer);
}