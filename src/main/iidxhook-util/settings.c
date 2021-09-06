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
        struct irp irp_altered;
        wchar_t redir_path[MAX_PATH];

        if (wstr_narrow(irp->open_filename, &log_str)) {
            log_misc("Remapped settings path %s", log_str);
            free(log_str);
        } else {
            log_warning("Cannot narrow path wstr for logging on settings remap match");
        }

        // Create a copy of the irp including a deep copy of the open_filename to avoid modifying
        // the original one.
        // Otherwise, state is altered which callers upwards in the call stack might depend
        // upon, breaking stuff in very unforseeable ways...
        wstr_cpy(redir_path, MAX_PATH, irp->open_filename);
        memcpy(&irp_altered, irp, sizeof(struct irp));
        irp_altered.open_filename = (const wchar_t*) &redir_path;

        // Apply redir to path copy
        redir_path[1] = L'\\';

        /* Create local settings folders if not available */
        if (!settings_folders_checked) {
            settings_folders_checked = true;

            for (char c = 'd'; c <= 'f'; c++) {
                char str[3];

                str[0] = c;
                str[1] = '\\';
                str[2] = '\0';

                if (!path_exists(str)) {
                    log_misc("Creating local settings folder %s\\", str);
                    
                    if (!real_CreateDirectoryA(str, NULL)) {
                        log_warning("Creating local settings folder failed: %s", str);
                    }
                }
            }
        }

        // Use altered copy of irp instead of irp from callers upstream
        return iohook_invoke_next(&irp_altered);
    }

    return iohook_invoke_next(irp);
}
