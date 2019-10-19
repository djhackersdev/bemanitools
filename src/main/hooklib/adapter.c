#include <iphlpapi.h>
#include <wincrypt.h> /* Required by mingw for some reason */
#include <windows.h>
#include <winsock2.h>

#include "hook/table.h"

#include "hooklib/adapter.h"

#include "util/codepage.h"
#include "util/defs.h"

static DWORD WINAPI
my_GetAdaptersInfo(PIP_ADAPTER_INFO adapter_info, PULONG out_buf_len);

static DWORD(WINAPI *real_GetAdaptersInfo)(
    PIP_ADAPTER_INFO adapter_info, PULONG out_buf_len);

static const struct hook_symbol adapter_hook_syms[] = {
    {.name = "GetAdaptersInfo",
     .patch = my_GetAdaptersInfo,
     .link = (void *) &real_GetAdaptersInfo},
};

static DWORD WINAPI
my_GetAdaptersInfo(PIP_ADAPTER_INFO adapter_info, PULONG out_buf_len)
{
    DWORD ret;
    PMIB_IPFORWARDTABLE ip_fwd_table;
    DWORD table_size;
    DWORD best_adapter;
    PIP_ADAPTER_INFO info;

    ret = real_GetAdaptersInfo(adapter_info, out_buf_len);

    if (ret != 0) {
        return ret;
    }

    ip_fwd_table = (MIB_IPFORWARDTABLE *) malloc(sizeof(MIB_IPFORWARDTABLE));
    table_size = 0;

    if (GetIpForwardTable(ip_fwd_table, &table_size, 1) ==
        ERROR_INSUFFICIENT_BUFFER) {
        free(ip_fwd_table);
        ip_fwd_table = (MIB_IPFORWARDTABLE *) malloc(table_size);
    }

    if (GetIpForwardTable(ip_fwd_table, &table_size, 1) != NO_ERROR ||
        ip_fwd_table->dwNumEntries == 0) {
        return ret;
    }

    best_adapter = ip_fwd_table->table[0].dwForwardIfIndex;
    free(ip_fwd_table);

    info = adapter_info;

    while (info) {
        if (info->Index == best_adapter) {
            memcpy(adapter_info, info, sizeof(*info));
            adapter_info->Next = 0;
            return ret;
        }
        info = info->Next;
    }

    return ret;
}

void adapter_hook_init(void)
{
    hook_table_apply(
        NULL, "iphlpapi.dll", adapter_hook_syms, lengthof(adapter_hook_syms));
}
