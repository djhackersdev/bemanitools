#define LOG_MODULE "api-hook"

#include <windows.h>

#include "api/core/config.h"
#include "api/core/log.h"
#include "api/core/thread.h"
#include "api/hook.h"
#include "api/lib.h"

#include "hook/pe.h"

#include "util/mem.h"
#include "util/str.h"

#define MM_ALLOCATION_GRANULARITY 0x10000

struct api_hook {
    api_lib_t lib;
    bt_hook_core_thread_impl_set_t core_thread_impl_set;
    bt_hook_core_log_impl_set_t core_log_impl_set;
    bt_hook_core_config_impl_set_t core_config_impl_set;
    bt_hook_iat_dll_name_get_t iat_dll_name_get;
    bt_hook_main_init_t main_init;
    bt_hook_main_fini_t main_fini;
    bt_hook_pre_avs_init_t pre_avs_init;
};

static void _api_hook_module_dll_iat(
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

static void _api_hook_resolve(api_hook_t *hook)
{
    hook->core_thread_impl_set =
        (bt_hook_core_thread_impl_set_t) api_lib_func_optional_resolve(
            hook->lib, "bt_hook_core_thread_impl_set", 1);
    hook->core_log_impl_set = (bt_hook_core_log_impl_set_t) api_lib_func_optional_resolve(
        hook->lib, "bt_hook_core_log_impl_set", 1);
    hook->core_config_impl_set = (bt_hook_core_config_impl_set_t) api_lib_func_optional_resolve(
        hook->lib, "bt_hook_core_config_impl_set", 1);
    hook->before_avs_init = (bt_hook_pre_avs_init_t) api_lib_func_optional_resolve(
        hook->lib, "bt_hook_pre_avs_init", 1);
    hook->iat_dll_name_get = (bt_hook_iat_dll_name_get_t) api_lib_func_optional_resolve(
        hook->lib, "bt_hook_iat_dll_name_get", 1);
    hook->main_init = (bt_hook_main_init_t) api_lib_func_optional_resolve(
        hook->lib, "bt_hook_main_init", 1);
    hook->main_fini = (bt_hook_main_fini_t) api_lib_func_optional_resolve(
        hook->lib, "bt_hook_main_fini", 1);
}

void api_hook_load(const char *path, api_hook_t **hook)
{
    log_assert(path);
    log_assert(hook);

    *hook = xmalloc(sizeof(api_hook_t));
    memset(hook, 0, sizeof(api_hook_t));

    api_lib_load(path, (*hook)->lib);
    _api_hook_resolve(hook);
}

void api_hook_free(api_hook_t **hook)
{
    log_assert(hook);

    api_lib_free((*hook)->lib);
    memset(*hook, 0, sizeof(api_hook_t));
}

void api_hook_core_thread_impl_set(const api_hook_t *hook)
{
    bt_core_thread_impl_t thread_impl;

    log_assert(hook);

    api_lib_func_pre_invoke_log(hook->lib, "core_thread_impl_set");

    if (hook->core_thread_impl_set) {
        api_core_thread_impl_get(&thread_impl);
        hook->core_thread_impl_set(&thread_impl);
    } else {
        log_warning("%s (%p): 'core_thread_impl_set' not implemented, ignore");
    }

    api_lib_func_post_invoke_log(hook->lib, "core_thread_impl_set");
}

void api_hook_core_log_impl_set(const api_hook_t *hook)
{
    bt_core_log_impl_t log_impl;

    log_assert(hook);

    api_lib_func_pre_invoke_log(hook->lib, "core_log_impl_set");

    if (hook->core_log_impl_set) {
        api_core_log_impl_get(&log_impl);
        hook->core_log_impl_set(&log_impl);
    } else {
        log_misc("%s (%p): 'core_log_impl_set' not implemented, ignore");
    }

    api_lib_func_post_invoke_log(hook->lib, "core_log_impl_set");
}

void api_hook_core_config_impl_set(const api_hook_t *hook)
{
    bt_core_config_impl_t config_impl;

    log_assert(hook);

    api_lib_func_pre_invoke_log(hook->lib, "core_config_impl_set");\

    if (hook->core_config_impl_set) {
        api_core_config_impl_get(&config_impl);
        hook->core_config_impl_set(&config_impl);
    } else {
        log_misc("%s (%p): 'core_config_impl_set' not implemented, ignore");
    }

    api_lib_func_post_invoke_log(hook->lib, "core_config_impl_set");
}

bool api_hook_pre_avs_init(
    const api_hook_t *hook, const core_config_t *config)
{
    bool result;

    log_assert(hook);
    log_assert(config);

    api_lib_func_pre_invoke_log(hook->lib, "pre_avs_init");

    if (hook->before_avs_init) {
        result = hook->pre_avs_init(config);
    } else {
        log_misc("%s (%p): 'pre_avs_init' not implemented, ignore");
        result = true;
    }

    api_lib_func_post_invoke_log(hook->lib, "pre_avs_init");

    return result;
}

void api_hook_iat_apply(const api_hook_t *hook, HMODULE game_module)
{
    const char *iat_dll_name;

    log_assert(hook);
    log_assert(game_module);

    api_lib_func_pre_invoke_log(hook->lib, "iat_dll_name_get");

    if (hook->iat_dll_name_get) {
        iat_dll_name = hook->iat_dll_name_get();

        _api_hook_module_dll_iat(game_module, iat_dll_name, hook->path);
    } else {
        log_misc("%s (%p): 'iat_dll_name_get' not implemented, ignore");
    }

    api_lib_func_post_invoke_log(hook->lib, "iat_dll_name_get");
}

bool api_hook_main_init(
    const api_hook_t *hook,
    HMODULE game_module,
    const core_config_t *config)
{
    bool result;

    log_assert(hook);
    log_assert(game_module);
    log_assert(config);

    api_lib_func_pre_invoke_log(hook->lib, "main_init");

    if (hook->main_init) {
        result = hook->main_init(game_module, config);
    } else {
        log_misc("%s (%p): 'main_init' not implemented, ignore");
        result = true;
    }

    api_lib_func_post_invoke_log(hook->lib, "main_init");

    return result;
}

void api_hook_main_fini(const api_hook_t *hook)
{
    log_assert(hook);

    api_lib_func_pre_invoke_log(hook->lib, "main_fini");

    if (hook->main_fini) {
        hook->main_fini();
    } else {
        log_misc("%s (%p): 'main_fini' not implemented, ignore");
    }

    api_lib_func_post_invoke_log(hook->lib, "main_fini");
}