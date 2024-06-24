#include <windows.h>

void win_error_log_fatal_on_last_error()
{
    HMODULE module;
    LPSTR buffer;
    DWORD err;

    err = GetLastError();

    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR) &buffer,
        0,
        NULL);

        if (err == ERROR_MOD_NOT_FOUND) {
            log_warning("%s is likely missing dependencies", path);
        }

        log_fatal("Failed to load module %s: %s", path, buffer);

        LocalFree(buffer);
    }

    log_misc("%s (%p): loaded", path, module);

    return module;
}