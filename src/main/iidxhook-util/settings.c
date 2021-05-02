#define LOG_MODULE "settings-hook"

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "hook/iohook.h"
#include "hook/table.h"

#include "util/defs.h"
#include "util/fs.h"
#include "util/log.h"
#include "util/str.h"

/* ------------------------------------------------------------------------- */

static BOOL WINAPI my_CreateDirectoryA(
    LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);

static BOOL(WINAPI *real_CreateDirectoryA)(
    LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);

/* ------------------------------------------------------------------------- */

static const struct hook_symbol settings_hook_syms[] = {
    {.name = "CreateDirectoryA",
     .patch = my_CreateDirectoryA,
     .link = (void **) &real_CreateDirectoryA},
};

static bool settings_folders_checked;

/* ------------------------------------------------------------------------- */

BOOL WINAPI my_CreateDirectoryA(
    LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    if (lpPathName != NULL &&
        (lpPathName[0] == 'd' || lpPathName[0] == 'e' ||
         lpPathName[0] == 'f') &&
        lpPathName[1] == ':') {
        char new_path[MAX_PATH];

        strcpy(new_path, lpPathName);
        new_path[1] = '\\';
        log_misc("(CreateDir) Remapped settings path %s", new_path);

        return real_CreateDirectoryA(new_path, lpSecurityAttributes);
    }

    return real_CreateDirectoryA(lpPathName, lpSecurityAttributes);
}

/* ------------------------------------------------------------------------- */

void settings_hook_init(void)
{
    hook_table_apply(
        NULL, "kernel32.dll", settings_hook_syms, lengthof(settings_hook_syms));

    log_info("Inserted settings hooks");
}

HRESULT
settings_hook_dispatch_irp(struct irp *irp)
{
    if (irp->op == IRP_OP_OPEN &&
        (irp->open_filename[0] == L'd' || irp->open_filename[0] == L'e' ||
         irp->open_filename[0] == L'f') &&
        irp->open_filename[1] == L':') {
        char *log_str;

        log_misc("settings hook trapped open call");

        ((wchar_t *) irp->open_filename)[1] = L'\\';

        if (wstr_narrow(irp->open_filename, &log_str)) {
            log_misc("Remapped settings path %s", log_str);
            free(log_str);
        } else {
            log_warning("Remapped settings path wstr_narrow failed");
        }

        log_misc("before settings_folders_checked");

        /* Create local settings folders if not available */
        if (!settings_folders_checked) {
            settings_folders_checked = true;

            log_misc("after settings_folders_checked");

            for (char c = 'd'; c <= 'f'; c++) {
                char str[3];

                str[0] = c;
                str[1] = '\\';
                str[2] = '\0';

                log_misc("before path_exists: %s", str);

                if (!path_exists(str)) {
                    log_misc("Creating local settings folder %s\\", str);
                    CreateDirectoryA(str, NULL);
                }

                log_misc("after path_exists: %s", str);
            }
        }

        log_misc("iohook_invoke_next");

        return iohook_invoke_next(irp);
    }

    return iohook_invoke_next(irp);
}
