#include <windows.h>

#include "hook/pe.h"

#include "imports/avs.h"
#include "imports/eapki.h"

#include "launcher/module.h"

#include "util/log.h"
#include "util/str.h"

#define MM_ALLOCATION_GRANULARITY 0x10000

static bool module_replace_dll_iat(HMODULE hModule, struct array *iat_hook_dlls)
{
    log_assert(hModule);
    log_assert(iat_hook_dlls);

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

void module_context_init(struct module_context *module, const char *path)
{
    log_assert(module != NULL);
    log_assert(path != NULL);

    module->dll = LoadLibrary(path);

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

    module->path = str_dup(path);
}

void module_context_init_with_iat_hooks(
    struct module_context *module,
    const char *path,
    struct array *iat_hook_dlls)
{
    log_assert(module != NULL);
    log_assert(path != NULL);

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

bool module_context_invoke_init(
    const struct module_context *module,
    char *sidcode,
    struct property_node *app_config)
{
    dll_entry_init_t init;

    log_assert(module != NULL);
    log_assert(sidcode != NULL);
    log_assert(app_config != NULL);

    init = (void *) GetProcAddress(module->dll, "dll_entry_init");

    if (init == NULL) {
        log_fatal(
            "%s: dll_entry_init not found. Is this a game DLL?", module->path);
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
            "%s: dll_entry_main not found. Is this a game DLL?", module->path);
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
