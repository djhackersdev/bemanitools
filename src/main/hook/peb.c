#include <windows.h>
#include <winternl.h>

#include <assert.h>

#include "hook/peb.h"

static const PEB *peb_get(void)
{
#ifdef _M_AMD64
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

    return CONTAINING_RECORD(node, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
}

const peb_dll_t *peb_dll_get_next(const peb_dll_t *dll)
{
    const PEB *peb;
    const LIST_ENTRY *node;

    assert(dll != NULL);

    peb = peb_get();
    node = dll->InMemoryOrderLinks.Flink;

    if (node == peb->Ldr->InMemoryOrderModuleList.Flink) {
        return NULL;
    }

    return CONTAINING_RECORD(node, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
}

HMODULE peb_dll_get_base(const peb_dll_t *dll)
{
    assert(dll != NULL);

    return dll->DllBase;
}
