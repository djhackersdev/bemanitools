#define LOG_MODULE "api-lib"

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

#include "util/mem.h"
#include "util/str.h"

#define API_LIB_FUNC_NAME_MAX_LEN 256

struct api_lib {
    char path[MAX_PATH];
    HMODULE module;
};

static HMODULE _api_lib_library_load(const char *path, bool resolve_references)
{
    HMODULE module;
    LPSTR buffer;
    DWORD err;

    log_misc("%s: loading", path);

    if (resolve_references) {
        module = LoadLibraryA(path);
    } else {
        module = LoadLibraryExA(path, NULL, DONT_RESOLVE_DLL_REFERENCES);
    }

    if (module == NULL) {
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

        log_fatal("Failed to load library %s: %s", path, buffer);

        LocalFree(buffer);
    }

    log_misc("%s (%p): loaded", path, module);

    return module;
}

void api_lib_load(const char *path, api_lib_t **lib)
{
    log_assert(path);
    log_assert(lib);

    *lib = xmalloc(sizeof(api_lib_t));
    memset(*lib, 0, sizeof(api_lib_t));

    log_info("%s: load", path);

    str_cpy((*lib)->path, sizeof((*lib)->path), path);
    (*lib)->module = _api_lib_library_load(path, true);

    log_misc("%s (%p): loaded", (*lib)->path, (*lib)->module);
}

HMODULE api_lib_module_get(const api_lib_t *lib)
{
    log_assert(lib);

    return lib->module;
}

const char *api_lib_path_get(const api_lib_t *lib)
{
    log_assert(lib);

    return lib->path;
}

void *api_lib_func_resolve(const api_lib_t *lib, const char *name, uint8_t version)
{
    char name_versioned[API_LIB_FUNC_NAME_MAX_LEN];
    char version[5];
    void *func;

    log_assert(lib);
    log_assert(name);
    // -3 to leave space for versioning postfix, i.e. "_255"
    log_assert(strlen(name) - 4 < API_LIB_FUNC_NAME_MAX_LEN);
    log_assert(version > 0);

    str_cpy(name_versioned, sizeof(name_versioned), name);

    if (version > 1) {
        sprintf(version, "_%d", version);
        str_cat(name_versioned, sizeof(name_versioned), version);
    }

    func = GetProcAddress((*lib)->module, name_versioned);

    if (!func) {
        log_fatal("Missing required function '%s' in library '%s'", name_versioned, (*lib)->path);
    }

    return func;
}

void *api_lib_func_optional_resolve(const api_lib_t *lib, const char *name, uint8_t version)
{
    char name_versioned[API_LIB_FUNC_NAME_MAX_LEN];
    char version[5];
    void *func;

    log_assert(lib);
    log_assert(name);
    // -3 to leave space for versioning postfix, i.e. "_255"
    log_assert(strlen(name) - 4 < API_LIB_FUNC_NAME_MAX_LEN);
    log_assert(version > 0);

    str_cpy(name_versioned, sizeof(name_versioned), name);

    if (version > 1) {
        sprintf(version, "_%d", version);
        str_cat(name_versioned, sizeof(name_versioned), version);
    }

    func = GetProcAddress((*lib)->module, name_versioned);

    return func;
}

void api_lib_func_pre_invoke_log(const api_lib_t *lib, const char *name)
{
    log_misc("%s (%p): >>> %s", lib->path, lib->module, name);    
}

void api_lib_func_post_invoke_log(const api_lib_t *lib, const char *name)
{
    log_misc("%s (%p): <<< %s", lib->path, lib->module, name);   
}

void api_lib_free(api_lib_t **lib)
{
    log_assert(lib);

    log_misc("%s (%p): free", (*lib)->path, (*lib)->module);

    FreeLibrary((*lib)->module);
    memset(*lib, 0, sizeof(api_lib_t));
}
