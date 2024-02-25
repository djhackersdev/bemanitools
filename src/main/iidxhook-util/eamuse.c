#define LOG_MODULE "eamuse-hook"

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "core/log.h"

#include "hook/table.h"

#include "iidxhook-util/eamuse.h"

#include "util/defs.h"
#include "util/str.h"

/* ------------------------------------------------------------------------- */

static unsigned long STDCALL my_inet_addr(const char *cp);
static int STDCALL
my_connect(SOCKET s, const struct sockaddr *addr, int addrlen);
static struct hostent FAR *STDCALL my_gethostbyname(const char *nameB);

static unsigned long(STDCALL *real_inet_addr)(const char *cp);
static int(STDCALL *real_connect)(
    SOCKET s, const struct sockaddr *addr, int addrlen);
static struct hostent FAR *(STDCALL *real_gethostbyname)(const char *nameB);
/* ------------------------------------------------------------------------- */

static const struct hook_symbol eamuse_hook_syms[] = {
    /*  WS2_32.DLL's SDK import lib generates ordinal imports, so these
       ordinals are a frozen aspect of the Win32 ABI. */

    {.name = "inet_addr",
     .ordinal = 11,
     .patch = my_inet_addr,
     .link = (void **) &real_inet_addr},
    {.name = "connect",
     .ordinal = 4,
     .patch = my_connect,
     .link = (void **) &real_connect},
    {.name = "gethostbyname",
     .ordinal = 52,
     .patch = my_gethostbyname,
     .link = (void **) &real_gethostbyname},
};

/* ------------------------------------------------------------------------- */

static struct net_addr eamuse_server_addr;
static struct net_addr eamuse_server_addr_resolved;

/* ------------------------------------------------------------------------- */

static unsigned long STDCALL my_inet_addr(const char *cp)
{
    char *tmp;

    /* for a stock machine connected to the eamuse router,
       the game wants to connect to the standard domain
       services.konami.eamuse.fun
       depending on the router you got, it will be services.hostname.my.router
       so we catch that and turn it into any ip we want */

    /* bugfix win10: don't just catch services.konami... because
       win10 is doing some weird stuff and this call also contains
       various IP addresses. Always return the server address */
    tmp = net_addr_to_str(&eamuse_server_addr_resolved);
    log_misc("my_inet_addr: '%s' -> %s", cp, tmp);
    free(tmp);

    return eamuse_server_addr_resolved.ipv4.addr;
}

static int STDCALL
my_connect(SOCKET s, const struct sockaddr *addr, int addrlen)
{
    char *tmp;

    if (addr->sa_family == AF_INET) {
        /* check if we got our server's address */
        struct sockaddr_in *addr_in = (struct sockaddr_in *) addr;

        if (addr_in->sin_addr.S_un.S_addr ==
            eamuse_server_addr_resolved.ipv4.addr) {
            tmp = net_addr_to_str(&eamuse_server_addr_resolved);
            log_misc(
                "Patching port '%d' of %s to %d",
                ntohs(addr_in->sin_port),
                tmp,
                eamuse_server_addr_resolved.ipv4.port);
            free(tmp);

            addr_in->sin_port = htons(eamuse_server_addr_resolved.ipv4.port);
        }
    }

    return real_connect(s, addr, addrlen);
}

static struct hostent FAR *STDCALL my_gethostbyname(const char *name)
{
    char *tmp;

    /* for doc, checkout the other detour of inetaddr above
       this call is used starting GOLD (not used on pre GOLD) */

    /* bugfix win10: don't just catch services.konami... because
       win10 is doing some weird stuff and this call also contains
       various IP addresses. Always return the server address */
    tmp = net_addr_to_str(&eamuse_server_addr_resolved);
    log_misc("my_gethostbyname: '%s' to ip %s", name, tmp);
    free(tmp);

    /* very ugly hack to get this working.
    don't bother with dynamic allocations and have stuff staticly alloc'd
    just populate what's touched by the game */
    static struct hostent ret;
    static uint32_t addr;
    static char *arr[2];
    static bool first = true;

    if (first) {
        ret.h_length = 4;
        ret.h_addr_list = (char **) &arr;
        ret.h_addr_list[0] = (char *) &addr;
        ret.h_addr_list[1] = NULL;
        ret.h_aliases = NULL;
        ret.h_name = NULL;
    }

    addr = eamuse_server_addr_resolved.ipv4.addr;

    return &ret;
}

/* ------------------------------------------------------------------------- */

void eamuse_hook_init(void)
{
    hook_table_apply(
        NULL, "ws2_32.dll", eamuse_hook_syms, lengthof(eamuse_hook_syms));

    log_info("Inserted eamuse hooks");
}

void eamuse_set_addr(const struct net_addr *addr)
{
    char *tmp_str;
    char *tmp_str2;

    log_assert(addr);

    memcpy(&eamuse_server_addr, addr, sizeof(struct net_addr));

    tmp_str = net_addr_to_str(&eamuse_server_addr);

    eamuse_server_addr_resolved.type = NET_ADDR_TYPE_IPV4;

    if (!net_resolve_hostname_net_addr(
            &eamuse_server_addr, &eamuse_server_addr_resolved.ipv4)) {
        log_fatal("Failed to resolve eamuse server address: %s", tmp_str);
        free(tmp_str);
        return;
    }

    tmp_str2 = net_addr_to_str(&eamuse_server_addr_resolved);

    // when resolving URLs with net_str_parse
    // it will default to NET_INVALID_PORT if no port is specified
    // assume default 80 here as a good sanity check
    if (eamuse_server_addr_resolved.ipv4.port == NET_INVALID_PORT) {
        log_info("No port was specified, using port 80 as fallback");
        eamuse_server_addr_resolved.ipv4.port = 80;
    }

    log_info("Target eamuse server %s resolved to %s", tmp_str, tmp_str2);

    free(tmp_str);
    free(tmp_str2);
}

void eamuse_check_connection()
{
    if (!net_check_remote_connection(&eamuse_server_addr_resolved, 5000)) {
        log_info("Target eamuse server reachable.");
    } else {
        log_warning(
            "Target eamuse server is not reachable. Game might throw "
            "network errors/warnings if eamuse is enabled.");
    }
}