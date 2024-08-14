#define LOG_MODULE "module"

#include <windows.h>

#include "core/log.h"

#include "hook/pe.h"

#include "imports/avs.h"
#include "imports/eapki.h"

#include "launcher/module.h"
#include "launcher/property-util.h"

#include "util/str.h"

#define MM_ALLOCATION_GRANULARITY 0x10000

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

static bool
module_replace_dll_iat(HMODULE hModule, const struct array *iat_hook_dlls)
{
    log_assert(hModule);
    log_assert(iat_hook_dlls);

    log_misc("replace dll iat: %p", hModule);

    if (iat_hook_dlls->nitems == 0)
        return true;

    PBYTE pbModule = (PBYTE) hModule;

    // Find EXE base in process memory
    IMAGE_DOS_HEADER *idh = (IMAGE_DOS_HEADER *) pbModule;
    IMAGE_NT_HEADERS *inh = (IMAGE_NT_HEADERS *) (pbModule + idh->e_lfanew);

    // Search through import table if it exists and replace the target DLL with
    // our DLL filename
    PIMAGE_SECTION_HEADER pRemoteSectionHeaders =
        (PIMAGE_SECTION_HEADER) ((PBYTE) pbModule + sizeof(inh->Signature) +
                                 sizeof(inh->FileHeader) +
                                 inh->FileHeader.SizeOfOptionalHeader);
    size_t total_size = inh->OptionalHeader.SizeOfHeaders;

    for (DWORD n = 0; n < inh->FileHeader.NumberOfSections; ++n) {
        IMAGE_SECTION_HEADER *header =
            (IMAGE_SECTION_HEADER *) (pRemoteSectionHeaders + n);
        size_t new_total_size =
            header->VirtualAddress + header->Misc.VirtualSize;
        if (new_total_size > total_size)
            total_size = new_total_size;
    }

    void *remote_addr = NULL;
    size_t remote_addr_ptr = 0;

    for (size_t i = 0; i < 10000 && remote_addr == NULL; i++) {
        remote_addr = VirtualAlloc(
            pbModule + total_size + (MM_ALLOCATION_GRANULARITY * (i + 1)),
            (MAX_PATH + 1) * iat_hook_dlls->nitems,
            MEM_RESERVE | MEM_COMMIT,
            PAGE_READWRITE);
    }

    log_assert(remote_addr != NULL);

    if (inh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]
            .VirtualAddress != 0) {
        PIMAGE_IMPORT_DESCRIPTOR pImageImport =
            (PIMAGE_IMPORT_DESCRIPTOR) (pbModule +
                                        inh->OptionalHeader
                                            .DataDirectory
                                                [IMAGE_DIRECTORY_ENTRY_IMPORT]
                                            .VirtualAddress);

        DWORD size = 0;
        while (inh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]
                       .Size == 0 ||
               size < inh->OptionalHeader
                          .DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]
                          .Size) {
            IMAGE_IMPORT_DESCRIPTOR *ImageImport =
                (IMAGE_IMPORT_DESCRIPTOR *) pImageImport;

            if (ImageImport->Name == 0) {
                break;
            }

            const char *name = (const char *) (pbModule + ImageImport->Name);

            for (size_t i = 0; i < iat_hook_dlls->nitems; i++) {
                const char *iat_hook_dll =
                    *array_item(char *, iat_hook_dlls, i);

                char *iat_hook_replacement = strstr(iat_hook_dll, "=");

                if (!iat_hook_replacement)
                    continue;

                *iat_hook_replacement = '\0';

                const char *expected_dll = iat_hook_dll;
                const char *replacement_path_dll = iat_hook_replacement + 1;

                if (strcmp(name, expected_dll) == 0) {
                    pe_patch(
                        (PBYTE) remote_addr + remote_addr_ptr,
                        replacement_path_dll,
                        strlen(replacement_path_dll));

                    log_misc(
                        "Replacing %s with %s", name, replacement_path_dll);

                    DWORD val = (DWORD) ((PBYTE) remote_addr - pbModule);
                    pe_patch(&ImageImport->Name, &val, sizeof(DWORD));
                    pe_patch(pImageImport, &ImageImport, sizeof(ImageImport));

                    remote_addr_ptr += strlen(replacement_path_dll) + 1;
                }

                *iat_hook_replacement = '=';
            }

            pImageImport++;
            size += sizeof(IMAGE_IMPORT_DESCRIPTOR);
        }
    } else {
        log_misc("Couldn't find import table, can't hook DLL\n");
        goto inject_fail;
    }

    return true;

inject_fail:
    return false;
}

void module_init(struct module_context *module, const char *path)
{
    log_assert(module != NULL);
    log_assert(path != NULL);

    log_info("init: %s", path);

    module->dll = LoadLibraryA(path);

    if (module->dll == NULL) {
        LPSTR buffer;
        DWORD err = GetLastError();

        FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            err,
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

        log_fatal("%s: Failed to load game DLL: %s", path, buffer);

        LocalFree(buffer);
    }

    module->path = str_dup(path);

    log_misc("init done");
}

void module_with_iat_hooks_init(
    struct module_context *module,
    const char *path,
    const struct array *iat_hook_dlls)
{
    log_assert(module != NULL);
    log_assert(path != NULL);

    log_info("init iat hooks: %s", path);

    module->dll = LoadLibraryExA(path, NULL, DONT_RESOLVE_DLL_REFERENCES);

    if (module->dll == NULL) {
        LPSTR buffer;
        DWORD err = GetLastError();

        FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            err,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR) &buffer,
            0,
            NULL);

        if (err == ERROR_MOD_NOT_FOUND) {
            log_warning("%s is likely missing dependencies", path);
            log_warning("Do you have vcredist/directx runtimes installed?");
        }

        log_fatal("%s: Failed to load game DLL: %s", path, buffer);

        LocalFree(buffer);
    }

    // Add IAT hooks
    module_replace_dll_iat(module->dll, iat_hook_dlls);

    log_misc("Finished processing IAT hooks");

    // Resolve all imports like a normally loaded DLL
    pe_resolve_imports(module->dll);

    dll_entry_t orig_entry = pe_get_entry_point(module->dll);
    orig_entry(module->dll, DLL_PROCESS_ATTACH, NULL);

    log_misc("Finished resolving imports");

    module->path = str_dup(path);
}

void module_init_invoke(
    const struct module_context *module,
    struct ea3_ident_config *ea3_ident_config,
    struct property_node *app_params_node)
{
    char sidcode_short[17];
    char sidcode_long[21];
    char security_code[9];
    dll_entry_init_t init;
    bool ok;

    log_info("init invoke");

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

    init = (void *) GetProcAddress(module->dll, "dll_entry_init");

    if (init == NULL) {
        log_fatal(
            "%s: dll_entry_init not found. Is this a game DLL?", module->path);
    }

    log_info("Invoking game init...");

    struct property *prop =
        property_util_cstring_load("<param><io>p3io</io></param>");

    struct property_node *prop_node = property_search(prop, NULL, "/param");

    property_util_log(prop);
    property_util_node_log(prop_node);

    ok = init(sidcode_short, prop_node);

    if (!ok) {
        log_fatal("%s: dll_module_init() returned failure", module->path);
    } else {
        log_info("Game init done");
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

    log_misc("init invoke done");
}

bool module_main_invoke(const struct module_context *module)
{
    /* GCC warns if you call a variable "main" */
    dll_entry_main_t main_;
    bool result;

    log_assert(module != NULL);

    log_info("main invoke");

    main_ = (void *) GetProcAddress(module->dll, "dll_entry_main");

    if (main_ == NULL) {
        log_fatal(
            "%s: dll_entry_main not found. Is this a game DLL?", module->path);
    }

    log_info("Invoking game's main function...");

    result = main_();

    log_info("Main terminated, result: %d", result);

    return result;
}

void module_fini(struct module_context *module)
{
    log_info("fini");

    if (module == NULL) {
        return;
    }

    free(module->path);

    if (module->dll != NULL) {
        FreeLibrary(module->dll);
    }

    log_misc("fini done");
}
