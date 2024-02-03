#define LOG_MODULE "procmon-lib"

#include <windows.h>

#include "procmon-lib/procmon.h"

#include "util/log.h"

#define PROCMON_LIB "procmon.dll"
#define CONCAT(x, y) x ## y
#define LOAD_FUNC(lib_func, module, path, func_name) \
    lib_func = (CONCAT(func_name, _t)) GetProcAddress(module, #func_name); \
    if (lib_func == NULL) { \
        log_fatal("Failed to load function '%s' from '%s'", #func_name, path); \
    }

bool procmon_available()
{
    HMODULE module;

    module = LoadLibraryExA(PROCMON_LIB, NULL, DONT_RESOLVE_DLL_REFERENCES);

    if (module == NULL) {
        return false;
    } else {
        FreeLibrary(module);
        return true;
    }
}

void procmon_init(struct procmon *procmon)
{
    log_assert(procmon);

    memset(procmon, 0, sizeof(*procmon));
}

void procmon_load(struct procmon *procmon)
{
    HMODULE module;
    uint32_t api_version;

    log_assert(procmon);

    module = LoadLibraryA(PROCMON_LIB);

    if (module == NULL) {
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

        log_fatal("Failed to load library %s: %s", PROCMON_LIB, buffer);

        LocalFree(buffer);
    }

    procmon->module = module;

    LOAD_FUNC(procmon->api_version, module, PROCMON_LIB, procmon_api_version);

    api_version = procmon->api_version();

    if (api_version != 0) {
        log_fatal("Unsupported API version %d of %s", api_version, PROCMON_LIB);
    }

    LOAD_FUNC(procmon->set_loggers, module, PROCMON_LIB, procmon_set_loggers);
    LOAD_FUNC(procmon->init, module, PROCMON_LIB, procmon_init);
    LOAD_FUNC(procmon->file_mon_enable, module, PROCMON_LIB, procmon_file_mon_enable);
    LOAD_FUNC(procmon->module_mon_enable, module, PROCMON_LIB, procmon_module_mon_enable);
    LOAD_FUNC(procmon->thread_mon_enable, module, PROCMON_LIB, procmon_thread_mon_enable);
}

void procmon_free(struct procmon *procmon)
{
    log_assert(procmon);

    FreeLibrary(procmon->module);
}