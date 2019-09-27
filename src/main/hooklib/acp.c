#include <windows.h>

#include <stdint.h>
#include <wchar.h>

#include <ntdef.h>
#include <ntstatus.h>

#include "hook/table.h"

#include "hooklib/acp.h"

#include "util/defs.h"
#include "util/codepage.h"
#include "util/log.h"

static NTSTATUS NTAPI my_RtlMultiByteToUnicodeN(
        wchar_t *dest, unsigned long destsz, unsigned long *out_destsz,
        const char *src, unsigned long srcsz);

static const struct hook_symbol acp_hook_syms[] = {
    {
        .name   = "RtlMultiByteToUnicodeN",
        .patch  = my_RtlMultiByteToUnicodeN,
    },
};

static NTSTATUS NTAPI my_RtlMultiByteToUnicodeN(
        wchar_t *dest, unsigned long destsz, unsigned long *out_destsz,
        const char *src, unsigned long srcsz)
{
    unsigned long result;

    result = MultiByteToWideChar(CP_SHIFT_JIS, 0, src, srcsz, dest, destsz);

    if (result == 0) {
        switch (GetLastError()) {
            case ERROR_INSUFFICIENT_BUFFER:
                return STATUS_BUFFER_TOO_SMALL;
            case ERROR_INVALID_FLAGS:
            case ERROR_INVALID_PARAMETER:
                return STATUS_INVALID_PARAMETER;
            case ERROR_NO_UNICODE_TRANSLATION:
                return STATUS_UNMAPPABLE_CHARACTER;
            default:
                return STATUS_UNSUCCESSFUL;
        }
    }

    if (out_destsz != NULL) {
        *out_destsz = result * sizeof(wchar_t);
    }

    return STATUS_SUCCESS;
}

void acp_hook_init(void)
{
    hook_table_apply(NULL, "ntdll.dll", acp_hook_syms, lengthof(acp_hook_syms));
    log_info("ACP Hook enabled");
}

