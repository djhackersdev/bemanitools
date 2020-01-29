#include <iphlpapi.h>
#include <wincrypt.h> /* Required by mingw for some reason */
#include <windows.h>
#include <winsock2.h>

#include <stdbool.h>
#include <string.h>

#include "hook/table.h"

#include "hooklib/adapter.h"

#include "util/codepage.h"
#include "util/defs.h"
#include "util/log.h"

static DWORD WINAPI
my_GetAdaptersInfo(PIP_ADAPTER_INFO adapter_info, PULONG out_buf_len);

static DWORD(WINAPI *real_GetAdaptersInfo)(
    PIP_ADAPTER_INFO adapter_info, PULONG out_buf_len);

static IP_ADDRESS_STRING override_address;
static bool use_address_override;

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

    info = adapter_info;

    if (use_address_override) {
        while (info) {
            // this is well defined to be at most sizeof(IP_ADDRESS_STRING)
            // and NULL filled if shorter (hence the memset in
            // adapter_hook_override)
            if (!memcmp(
                    info->IpAddressList.IpAddress.String,
                    override_address.String,
                    sizeof(IP_ADDRESS_STRING))) {
                log_info(
                    "%s: using [override] adapter: %s, %s, %s, %s",
                    __FUNCTION__,
                    info->AdapterName,
                    info->Description,
                    info->IpAddressList.IpAddress.String,
                    info->IpAddressList.IpMask.String);

                // copy only this adapter over
                memcpy(adapter_info, info, sizeof(*info));
                adapter_info->Next = 0;
                return ret;
            }
            info = info->Next;
        }
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
            log_info(
                "%s: using [best] adapter: %s, %s, %s, %s",
                __FUNCTION__,
                info->AdapterName,
                info->Description,
                info->IpAddressList.IpAddress.String,
                info->IpAddressList.IpMask.String);

            // copy only this adapter over
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

void adapter_hook_override(const char *adapter_address)
{
    // starts off false anyways due to static
    // but in case it gets called multiple times, set it anyways
    use_address_override = false;

    if (adapter_address == NULL || *adapter_address == NULL) {
        // empty, do nothing
        return;
    }

    if (strlen(adapter_address) > sizeof(IP_ADDRESS_STRING)) {
        log_warning(
            "%s: %s is not an ipv4 address", __FUNCTION__, adapter_address);
        return;
    }

    memset(override_address.String, 0, sizeof(IP_ADDRESS_STRING));
    memcpy(override_address.String, adapter_address, sizeof(IP_ADDRESS_STRING));

    use_address_override = true;
}