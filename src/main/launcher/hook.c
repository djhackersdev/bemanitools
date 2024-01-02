#define LOG_MODULE "hook"

#include <windows.h>

#include "launcher/hook.h"

#include "util/log.h"

void hook_load_dll(const char *path)
{
    log_assert(path);

    log_info("Load hook dll: %s", path);

    if (LoadLibraryA(path) == NULL) {
        LPSTR buffer;

        FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR) &buffer,
            0,
            NULL);

        log_fatal("%s: Failed to load hook DLL: %s", path, buffer);

        LocalFree(buffer);
    }

    log_misc("Load hook dll done");
}