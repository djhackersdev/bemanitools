#include <windows.h>

#include "imports/avs.h"
#include "imports/eapki.h"

#include "launcher/module.h"

#include "util/log.h"
#include "util/str.h"

void module_context_init(struct module_context *module, const char *path)
{
    log_assert(module != NULL);
    log_assert(path != NULL);

    module->dll = LoadLibrary(path);

    if (module->dll == NULL) {
        LPSTR buffer;

        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR) &buffer, 0, NULL);

        log_fatal("%s: Failed to load game DLL: %s", path, buffer);

        LocalFree(buffer);
    }

    module->path = str_dup(path);
}

bool module_context_invoke_init(const struct module_context *module,
        char *sidcode, struct property_node *app_config)
{
    dll_entry_init_t init;

    log_assert(module != NULL);
    log_assert(sidcode != NULL);
    log_assert(app_config != NULL);

    init = (void *) GetProcAddress(module->dll, "dll_entry_init");

    if (init == NULL) {
        log_fatal(
                "%s: dll_entry_init not found. Is this a game DLL?",
                module->path);
    }

    return init(sidcode, app_config);
}

bool module_context_invoke_main(const struct module_context *module)
{
    /* GCC warns if you call a variable "main" */
    dll_entry_main_t main_;

    log_assert(module != NULL);

    main_ = (void *) GetProcAddress(module->dll, "dll_entry_main");

    if (main_ == NULL) {
        log_fatal(
                "%s: dll_entry_main not found. Is this a game DLL?",
                module->path);
    }

    return main_();
}

void module_context_fini(struct module_context *module)
{
    if (module == NULL) {
        return;
    }

    free(module->path);

    if (module->dll != NULL) {
        FreeLibrary(module->dll);
    }
}

