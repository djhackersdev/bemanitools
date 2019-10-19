#define LOG_MODULE "eamuse-hook"

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "hook/table.h"

#include "util/defs.h"
#include "util/log.h"
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

static const struct hook_symbol eamuse_hook_syms[] = {
    {
        .name = "getaddrinfo",
        .ordinal = 176,
        .patch = my_getaddrinfo,
        .link = (void **) &real_getaddrinfo,
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

void jbhook_eamuse_hook_init(void)
{
    hook_table_apply(
        NULL, "ws2_32.dll", eamuse_hook_syms, lengthof(eamuse_hook_syms));

    log_info("Inserted eamuse hooks");
}
