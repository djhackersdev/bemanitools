#define LOG_MODULE "eamuse-hook"

#include <iphlpapi.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "hook/table.h"

#include "iface-core/log.h"

#include "util/defs.h"
#include "util/net.h"
#include "util/str.h"

static const char *jbhook_eamuse_konami_url = "eamuse.konami.";

static int(STDCALL *real_getaddrinfo)(
    PCSTR pNodeName,
    PCSTR pServiceName,
    const ADDRINFOA *pHints,
    PADDRINFOA *ppResult);

static int STDCALL my_getaddrinfo(
    PCSTR pNodeName,
    PCSTR pServiceName,
    const ADDRINFOA *pHints,
    PADDRINFOA *ppResult);

static DWORD WINAPI
my_GetIpAddrTable(PMIB_IPADDRTABLE pIpAddrTable, PULONG pdwSize, BOOL bOrder);

static const struct hook_symbol eamuse_hook_syms[] = {
    {
        .name = "getaddrinfo",
        .ordinal = 176,
        .patch = my_getaddrinfo,
        .link = (void **) &real_getaddrinfo,
    },
};

static const struct hook_symbol network_hook_syms[] = {
    {
        .name = "GetIpAddrTable",
        .patch = my_GetIpAddrTable,
    },
};

static int STDCALL my_getaddrinfo(
    PCSTR pNodeName,
    PCSTR pServiceName,
    const ADDRINFOA *pHints,
    PADDRINFOA *ppResult)
{
    log_misc("my_getaddrinfo: %s, %s", pNodeName, pServiceName);

    /* resolve eamuse.konami.fun/com to 127.0.0.1 to avoid lag spikes every
       150 seconds which might happen in-game as well */
    if (!strncmp(
            pNodeName,
            jbhook_eamuse_konami_url,
            strlen(jbhook_eamuse_konami_url))) {
        log_info("Resolve konami.eamuse.XXX -> localhost");
        pNodeName = "localhost";
    }

    return real_getaddrinfo(pNodeName, pServiceName, pHints, ppResult);
}

static DWORD get_best_ip(void)
{
    PIP_ADAPTER_INFO info = NULL;
    struct net_addr addr;
    // fallback to a very obviously wrong value
    DWORD ret = (5 << 24) | (7 << 16) | (3 << 8) | (0 << 0);
    ULONG sz = 0;

    // this uses our hooked GetAdaptersInfo from hooklib/adapter.c, thus using
    // the preferred interface instead of the default order
    log_assert(GetAdaptersInfo(info, &sz) == ERROR_BUFFER_OVERFLOW);

    info = malloc(sz);

    if (GetAdaptersInfo(info, &sz) != NO_ERROR) {
        goto CLEANUP;
    }

    if (!net_str_parse(info->IpAddressList.IpAddress.String, &addr)) {
        goto CLEANUP;
    }

    // MS docs say this should be in network byte order, but we need to return
    // host byte order for the game to not throw the IP error
    ret = addr.ipv4.addr;

CLEANUP:
    free(info);

    return ret;
}

/**
 * In new games via network.dll:network_addr_is_changed and in old games
 * directly, this function is called to compare the current IP address vs the IP
 * address when the game was first loaded. If the two are different, an error is
 * printed. It can be dismissed but will show up every ~10 seconds, impeding
 * gameplay. Because GetAdaptersInfo is hooked to return a (sometimes) different
 * adapter, computers with multiple interfaces will often fail this comparison
 * check. By also hooking this function, the IP addresses of the two calls will
 * agree.
 */
static DWORD WINAPI
my_GetIpAddrTable(PMIB_IPADDRTABLE pIpAddrTable, PULONG pdwSize, BOOL bOrder)
{
    ULONG in = *pdwSize;
    // 1 element return table
    *pdwSize = sizeof(MIB_IPADDRTABLE) + sizeof(MIB_IPADDRROW);

    if (in < *pdwSize) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    pIpAddrTable->dwNumEntries = 1;
    pIpAddrTable->table[0].dwAddr = get_best_ip();

    return NO_ERROR;
}

void jbhook_util_eamuse_hook_init(void)
{
    hook_table_apply(
        NULL, "ws2_32.dll", eamuse_hook_syms, lengthof(eamuse_hook_syms));
    hook_table_apply(
        NULL, "iphlpapi.dll", network_hook_syms, lengthof(network_hook_syms));

    log_info("Inserted eamuse hooks");
}
