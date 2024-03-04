#define LOG_MODULE "module"

#include <windows.h>

#include "btapi/hook.h"

#include "core/log.h"

#include "hook/pe.h"

#include "imports/avs.h"
#include "imports/eapki.h"

#include "launcher/module.h"
#include "launcher/property-util.h"

#include "util/str.h"

struct module {
    char path[MAX_PATH];
    HMODULE module;
    dll_entry_init_t init;
    dll_entry_main_t main;
};

static bool _module_dependency_available(const char *lib)
{
    HMODULE module;

    module = LoadLibraryA(lib);

    if (module == NULL) {
        return false;
    } else {
        FreeLibrary(module);
        return true;
    }
}

HMODULE _module_load(const char *path, bool resolve_references)
{
    HMODULE module;
    LPSTR buffer;
    DWORD err;

    log_misc("Loading game module: %s", path);

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
            log_warning("Do you have vcredist/directx runtimes installed?");
            log_warning(
                "Ensure the installed dependencies match the architecture, "
                "32-bit/64-bit, of the game");
            log_warning(
                "Running heuristic for commonly used libraries (actual "
                "requirements depend on game)...");

            if (_module_dependency_available("d3d9.dll")) {
                log_warning("Could not find directx9 runtime");
            }

            if (_module_dependency_available("msvcr100.dll")) {
                log_warning("Could not find vcredist 2010 runtime");
            }

            if (_module_dependency_available("msvcr120.dll")) {
                log_warning("Could not find vcredist 2013 runtime");
            }

            if (_module_dependency_available("msvcp140.dll")) {
                log_warning("Could not find vcredist 2015 runtime");
            }
        }

        log_fatal("%s: Failed to load game module: %s", path, buffer);

        LocalFree(buffer);
    }

    log_misc("Loading game module done");

    return module;
}

static void _module_api_resolve(struct module *module)
{
    log_assert(module);

    module->init = (dll_entry_init_t) GetProcAddress(module->module, "dll_entry_init");

    if (!module->init) {
         log_fatal(
            "%s (%p): 'dll_entry_init' not found. Is this a game DLL?", module->path, module->module);
    }

    module->main = (dll_entry_main_t) GetProcAddress(module->main, "dll_entry_main");

    if (!module->main) {
        log_fatal(
            "%s (%p): dll_entry_main not found. Is this a game DLL?", module->path, module->module);
    }
}

void module_load(const char *path, struct module *module)
{
    log_assert(path != NULL);
    log_assert(module != NULL);

    log_info("%s: load", path);

    str_cpy(module->path, sizeof(module->path), path);
    module->module = _module_load(path, true);

    _module_api_resolve(module);

    log_misc("%s (%p): loaded", module->path, module->module);
}

void module_unresolved_load(
    const char *path,
    struct module *module)
{
    log_assert(module != NULL);
    log_assert(path != NULL);

    log_info("%s: unresolved load", path);

    str_cpy(module->path, sizeof(module->path), path);
    module->module = _module_load(path, false);
    _module_api_resolve(module);

    log_misc("%s (%p): unresolved loaded", module->path, module->module);

    return module->module;
}

HMODULE module_handle_get(const struct module *module)
{
    log_assert(module);

    return module->module;
}

void module_resolve(const struct module *module);
{
    log_assert(module);

    log_info("%s (%p) resolving", module->path, module->module);
    // Resolve all imports like a normally loaded DLL
    pe_resolve_imports(module->module);

    dll_entry_t orig_entry = pe_get_entry_point(module->module);

    log_misc("%s (%p): >>> DllMain");

    orig_entry(module->dll, DLL_PROCESS_ATTACH, NULL);

    log_misc("%s (%p): <<< DllMain");

    log_misc("%s (%p) resolved", module->path, module->module);
}

void module_init_invoke(
    const struct module *module,
    struct ea3_ident_config *ea3_ident_config,
    struct property_node *app_params_node)
{
    char sidcode_short[17];
    char sidcode_long[21];
    char security_code[9];
    bool ok;

    log_info("%s (%p): init invoke", module->path, module->module);

    /* Set up security env vars */

    str_format(
        security_code,
        lengthof(security_code),
        "G*%s%s%s%s",
        ea3_ident_config->model,
        ea3_ident_config->dest,
        ea3_ident_config->spec,
        ea3_ident_config->rev);

    log_misc("security code: %s", security_code);

    std_setenv("/env/boot/version", "0.0.0");
    std_setenv("/env/profile/security_code", security_code);
    std_setenv("/env/profile/system_id", ea3_ident_config->pcbid);
    std_setenv("/env/profile/account_id", ea3_ident_config->pcbid);
    std_setenv("/env/profile/license_id", ea3_ident_config->softid);
    std_setenv("/env/profile/software_id", ea3_ident_config->softid);
    std_setenv("/env/profile/hardware_id", ea3_ident_config->hardid);

    /* Set up the short sidcode string, let dll_entry_init mangle it */

    str_format(
        sidcode_short,
        lengthof(sidcode_short),
        "%s%s%s%s%s",
        ea3_ident_config->model,
        ea3_ident_config->dest,
        ea3_ident_config->spec,
        ea3_ident_config->rev,
        ea3_ident_config->ext);

    log_misc("sidcode short: %s", sidcode_short);

    /* Set up long-form sidcode env var */

    str_format(
        sidcode_long,
        lengthof(sidcode_long),
        "%s:%s:%s:%s:%s",
        ea3_ident_config->model,
        ea3_ident_config->dest,
        ea3_ident_config->spec,
        ea3_ident_config->rev,
        ea3_ident_config->ext);

    log_misc("sidecode long: %s", sidcode_long);

    /* Set this up beforehand, as certain games require it in dll_entry_init */

    std_setenv("/env/profile/soft_id_code", sidcode_long);

    log_info("Invoking game init...");

    log_misc("%s (%p) >>> init", module->path, module->module);

    ok = module->init(sidcode_short, app_params_node);

    log_misc("%s (%p) <<< init: %d", module->path, module->module, ok);

    if (!ok) {
        log_fatal("%s: dll_module_init() returned failure", module->path);
    }

    /* Back-propagate sidcode, as some games modify it during init */

    memcpy(
        ea3_ident_config->model,
        sidcode_short + 0,
        sizeof(ea3_ident_config->model) - 1);
    ea3_ident_config->dest[0] = sidcode_short[3];
    ea3_ident_config->spec[0] = sidcode_short[4];
    ea3_ident_config->rev[0] = sidcode_short[5];
    memcpy(
        ea3_ident_config->ext,
        sidcode_short + 6,
        sizeof(ea3_ident_config->ext));

    /* Set up long-form sidcode env var again */

    str_format(
        sidcode_long,
        lengthof(sidcode_long),
        "%s:%s:%s:%s:%s",
        ea3_ident_config->model,
        ea3_ident_config->dest,
        ea3_ident_config->spec,
        ea3_ident_config->rev,
        ea3_ident_config->ext);

    std_setenv("/env/profile/soft_id_code", sidcode_long);

    log_misc("back-propagated sidcode long: %s", sidcode_long);

    log_misc("%s (%p): init invoked", module->path, module->module);
}

bool module_main_invoke(const struct module *module)
{
    bool result;

    log_assert(module != NULL);

    log_info("%s (%p): main invoke", module->path, module->module);

    log_misc("%s (%p) >>> main", module->path, module->module);

    result = module->main();

    log_misc("%s (%p) <<< main: %d", module->path, module->module, result);

    return result;
}

void module_free(struct module *module)
{
    log_assert(module);

    log_misc("%s (%p): free", module->path, module->module);

    FreeLibrary(module->module);
    memset(module, 0, sizeof(struct module));
}
