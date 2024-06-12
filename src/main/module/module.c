#define LOG_MODULE "module"

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

#include "hook/pe.h"

#include "iface-core/log.h"

#include "module/module.h"

#include "util/mem.h"
#include "util/str.h"

#define MM_ALLOCATION_GRANULARITY 0x10000
#define MODULE_FUNC_NAME_MAX_LEN 256

struct module {
    char path[MAX_PATH];
    HMODULE handle;
};

static void _module_hook_dll_iat(
    HMODULE hModule, const char *source_dll, const char *iat_hook)
{
    log_assert(hModule);
    log_assert(source_dll);
    log_assert(iat_hook);

    log_misc(
        "replace dll iat of module %p, %s -> %s",
        hModule,
        source_dll,
        iat_hook);

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
            MAX_PATH + 1,
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
            const char *expected_dll = source_dll;
            const char *replacement_path_dll = iat_hook;

            if (strcmp(name, expected_dll) == 0) {
                pe_patch(
                    (PBYTE) remote_addr + remote_addr_ptr,
                    replacement_path_dll,
                    strlen(replacement_path_dll));

                log_misc("Replacing %s with %s", name, replacement_path_dll);

                DWORD val = (DWORD) ((PBYTE) remote_addr - pbModule);

                pe_patch(&ImageImport->Name, &val, sizeof(DWORD));
                pe_patch(pImageImport, &ImageImport, sizeof(ImageImport));

                remote_addr_ptr += strlen(replacement_path_dll) + 1;
            }

            pImageImport++;
            size += sizeof(IMAGE_IMPORT_DESCRIPTOR);
        }
    } else {
        log_fatal("Couldn't find import table, can't hook DLL: %s", iat_hook);
    }
}

static HMODULE _module_load(const char *path, bool resolve_references)
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

        log_fatal("Failed to load module %s: %s", path, buffer);

        LocalFree(buffer);
    }

    log_misc("%s (%p): loaded", path, module);

    return module;
}

void module_load(const char *path, module_t **module)
{
    log_assert(path);
    log_assert(module);

    *module = xmalloc(sizeof(module_t));
    memset(*module, 0, sizeof(module_t));

    log_info("%s: load", path);

    str_cpy((*module)->path, sizeof((*module)->path), path);
    (*module)->handle = _module_load(path, true);

    log_misc("%s (%p): loaded", (*module)->path, (*module)->handle);
}

HMODULE module_handle_get(const module_t *module)
{
    log_assert(module);

    return module->handle;
}

const char *module_path_get(const module_t *module)
{
    log_assert(module);

    return module->path;
}

void *module_func_required_resolve(const module_t *module, const char *name)
{
    void *func;

    log_assert(module);
    log_assert(name);

    func = GetProcAddress(module->handle, name);

    module_func_required_verify(module, func, name);

    return func;
}

void *module_func_optional_resolve(const module_t *module, const char *name)
{
    void *func;

    log_assert(module);
    log_assert(name);

    func = GetProcAddress(module->handle, name);

    module_func_optional_verify(module, func, name);

    return func;
}

void module_func_required_verify(
    const module_t *module, void *func, const char *name)
{
    log_assert(module);
    log_assert(name);

    if (!func) {
        log_fatal(
            "%s (%p): Missing required function '%s'",
            module->path,
            module->handle,
            name);
    }

    log_misc(
        "%s (%p): required function '%s' at %p",
        module->path,
        module->handle,
        name,
        func);
}

void module_func_optional_verify(
    const module_t *module, void *func, const char *name)
{
    log_assert(module);
    log_assert(name);

    if (!func) {
        log_misc(
            "%s (%p): optional function '%s' NOT IMPLEMENTED",
            module->path,
            module->handle,
            name);
    } else {
        log_misc(
            "%s (%p): optional function '%s' at %p",
            module->path,
            module->handle,
            name,
            func);
    }
}

void module_func_pre_invoke_log(const module_t *module, const char *name)
{
    log_misc("%s (%p): >>> %s", module->path, module->handle, name);
}

void module_func_post_invoke_log(const module_t *module, const char *name)
{
    log_misc("%s (%p): <<< %s", module->path, module->handle, name);
}

void module_free(module_t **module)
{
    log_assert(module);

    log_misc("%s (%p): free", (*module)->path, (*module)->handle);

    FreeLibrary((*module)->handle);
    memset(*module, 0, sizeof(module_t));
}
