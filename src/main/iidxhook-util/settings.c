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
#include "util/mem.h"
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
static char settings_path[MAX_PATH] = ".\\";

/* ------------------------------------------------------------------------- */

static void settings_build_new_path(const char* orig_path, char* new_path, size_t new_path_len)
{
    size_t settings_path_len;

    settings_path_len = strlen(settings_path);

    log_assert(settings_path_len + strlen(orig_path) < new_path_len);

    strcpy(new_path, settings_path);
    strcat(new_path, orig_path);

    /* Remove : of drive letter. Multiple \ are fine and handled by windows */
    new_path[settings_path_len + 1] = '\\';
}

/* ------------------------------------------------------------------------- */

BOOL WINAPI my_CreateDirectoryA(
    LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    if (lpPathName != NULL &&
        (lpPathName[0] == 'd' || lpPathName[0] == 'e' ||
         lpPathName[0] == 'f') &&
        lpPathName[1] == ':') {
        char new_path[MAX_PATH];

        settings_build_new_path(lpPathName, new_path, sizeof(new_path));

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

    log_info("Inserted settings hooks, settings path: %s", settings_path);
}

void settings_hook_set_path(const char* path)
{    
    size_t len;

    len = strlen(path);

    log_assert(path > 0);
    log_assert(len < MAX_PATH);

    strcpy(settings_path, path);

    /* Ensure trailing \\ to allow simple concatinations in hooks */
    if (settings_path[len - 1] != '\\') {
        log_assert(len + 1 < MAX_PATH);

        settings_path[len] = '\\';
        settings_path[len + 1] = '\0';
    }

    log_info("Settings path: %s", settings_path);
}

HRESULT
settings_hook_dispatch_irp(struct irp *irp)
{
    if (irp->op == IRP_OP_OPEN &&
            (irp->open_filename[0] == L'd' || irp->open_filename[0] == L'e' ||
            irp->open_filename[0] == L'f') &&
            irp->open_filename[1] == L':') {
        HRESULT result;
        char new_path[MAX_PATH];
        const wchar_t *old_filename_wstr;
        wchar_t *filename_wstr;
        char *filename_cstr;

        log_assert(wstr_narrow(irp->open_filename, &filename_cstr));

        settings_build_new_path(filename_cstr, new_path, sizeof(new_path));

        free(filename_cstr);

        log_misc("Remapped settings path to %s", new_path);

        filename_wstr = str_widen(new_path);

        /* Temporarily swap open_filename */
        old_filename_wstr = irp->open_filename;
        irp->open_filename = filename_wstr;

        /* Create settings folders if not available */
        if (!settings_folders_checked) {
            settings_folders_checked = true;

            for (char c = 'd'; c <= 'f'; c++) {
                char new_path_folder[MAX_PATH];
                size_t settings_path_len = strlen(settings_path);

                log_assert(settings_path_len + 3 < MAX_PATH);

                strcpy(new_path_folder, settings_path);

                new_path_folder[settings_path_len] = c;
                new_path_folder[settings_path_len + 1] = '\\';
                new_path_folder[settings_path_len + 2] = '\0';

                if (!path_exists(new_path_folder)) {
                    log_misc("Creating local settings folder %s", new_path_folder);
                    CreateDirectoryA(new_path_folder, NULL);
                }
            }
        }

        result = iohook_invoke_next(irp);

        /* Revert to original irp */
        irp->open_filename = old_filename_wstr;
        free(filename_wstr);

        return result;
    }

    return iohook_invoke_next(irp);
}
