#include <windows.h>
#include <winternl.h>

#include "hook/peb.h"

#include "util/defs.h"
#include "util/str.h"

static const PEB *peb_get(void)
{
#ifdef __amd64
    return (const PEB *) __readgsqword(0x60);
#else
    return (const PEB *) __readfsdword(0x30);
#endif

}

const peb_dll_t *peb_dll_get_first(void)
{
    const PEB *peb;
    const LIST_ENTRY *node;

    peb = peb_get();
    node = peb->Ldr->InMemoryOrderModuleList.Flink;

    return containerof(node, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
}

const peb_dll_t *peb_dll_get_next(const peb_dll_t *dll)
{
    const PEB *peb;
    const LIST_ENTRY *node;

    peb = peb_get();
    node = dll->InMemoryOrderLinks.Flink;

    if (node == peb->Ldr->InMemoryOrderModuleList.Flink) {
        return NULL;
    }

    return containerof(node, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
}

HMODULE peb_dll_get_base(const peb_dll_t *dll)
{
    return dll->DllBase;
}

char *peb_dll_dup_name(const peb_dll_t *dll)
{
    const UNICODE_STRING *wstr;
    char *name;
    size_t i;

    wstr = &dll->FullDllName;

    for (i = wstr->Length / 2 - 1 ; i > 0 ; i--) {
        if (wstr->Buffer[i] == L'\\') {
            wstr_narrow(&wstr->Buffer[i + 1], &name);

            return name;
        }
    }

    return NULL;
}

