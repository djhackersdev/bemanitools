#define LOG_MODULE "app"

#include <windows.h>

#include "avs-ext/property-node.h"

#include "hook/pe.h"

#include "iface-core/log.h"

#include "launcher/app.h"

#include "util/str.h"

static bool _app_dependency_available(const char *lib)
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

HMODULE _app_load(const char *path, bool resolve_references)
{
    HMODULE module;
    LPSTR buffer;
    DWORD err;

    log_misc("Loading app module: %s", path);

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

            if (_app_dependency_available("d3d9.dll")) {
                log_warning("Could not find directx9 runtime");
            }

            if (_app_dependency_available("msvcr100.dll")) {
                log_warning("Could not find vcredist 2010 runtime");
            }

            if (_app_dependency_available("msvcr120.dll")) {
                log_warning("Could not find vcredist 2013 runtime");
            }

            if (_app_dependency_available("msvcp140.dll")) {
                log_warning("Could not find vcredist 2015 runtime");
            }
        }

        log_fatal("%s: Failed to load app module: %s", path, buffer);

        LocalFree(buffer);
    }

    log_misc("Loading app module done");

    return module;
}

static void _app_api_resolve(app_t *app)
{
    log_assert(app);

    app->init =
        (dll_entry_init_t) GetProcAddress(app->module, "dll_entry_init");

    if (!app->init) {
        log_fatal(
            "%s (%p): 'dll_entry_init' not found. Is this a game DLL?",
            app->path,
            app->module);
    }

    app->main =
        (dll_entry_main_t) GetProcAddress(app->module, "dll_entry_main");

    if (!app->main) {
        log_fatal(
            "%s (%p): dll_entry_main not found. Is this a game DLL?",
            app->path,
            app->module);
    }
}

void app_load(const char *path, app_t *app)
{
    log_assert(path != NULL);
    log_assert(app != NULL);

    log_info("%s: load", path);

    str_cpy(app->path, sizeof(app->path), path);
    app->module = _app_load(path, true);

    _app_api_resolve(app);

    log_misc("%s (%p): loaded", app->path, app->module);
}

void app_unresolved_load(const char *path, app_t *app)
{
    log_assert(path != NULL);
    log_assert(app != NULL);

    log_info("%s: unresolved load", path);

    str_cpy(app->path, sizeof(app->path), path);
    app->module = _app_load(path, false);
    _app_api_resolve(app);

    log_misc("%s (%p): unresolved loaded", app->path, app->module);
}

HMODULE app_module_handle_get(const app_t *app)
{
    log_assert(app);

    return app->module;
}

void app_resolve(const app_t *app)
{
    log_assert(app);

    log_info("%s (%p) resolving", app->path, app->module);
    // Resolve all imports like a normally loaded DLL
    pe_resolve_imports(app->module);

    dll_entry_t orig_entry = pe_get_entry_point(app->module);

    log_misc("%s (%p): >>> DllMain");

    orig_entry(app->module, DLL_PROCESS_ATTACH, NULL);

    log_misc("%s (%p): <<< DllMain");

    log_misc("%s (%p) resolved", app->path, app->module);
}

void app_init_invoke(
    const app_t *app,
    struct ea3_ident_config *ea3_ident_config,
    const core_property_node_t *app_params_node)
{
    char sidcode_short[17];
    char sidcode_long[21];
    char security_code[9];
    struct property_node *app_params_node_avs;
    bool ok;

    log_info("%s (%p): init invoke", app->path, app->module);

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

    log_misc("%s (%p) >>> init", app->path, app->module);

    app_params_node_avs =
        avs_ext_property_node_avs_property_node_get(app_params_node);

    ok = app->init(sidcode_short, app_params_node_avs);

    log_misc("%s (%p) <<< init: %d", app->path, app->module, ok);

    if (!ok) {
        log_fatal("%s: dll_entry_init_t() returned failure", app->path);
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

    log_misc("%s (%p): init invoked", app->path, app->module);
}

bool app_main_invoke(const app_t *app)
{
    bool result;

    log_assert(app != NULL);

    log_info("%s (%p): main invoke", app->path, app->module);

    log_misc("%s (%p) >>> main", app->path, app->module);

    result = app->main();

    log_misc("%s (%p) <<< main: %d", app->path, app->module, result);

    return result;
}

void app_free(app_t *app)
{
    log_assert(app);

    log_misc("%s (%p): free", app->path, app->module);

    FreeLibrary(app->module);
    memset(app, 0, sizeof(app_t));
}
