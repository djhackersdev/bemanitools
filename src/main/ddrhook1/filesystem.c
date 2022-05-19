#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hook/table.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/mem.h"

static BOOL STDCALL my_SetCurrentDirectoryA(LPCTSTR lpPathName);
static BOOL(STDCALL *real_SetCurrentDirectoryA)(LPCTSTR lpPathName);

static const struct hook_symbol filesystem_hook_syms[] = {
    {
        .name = "SetCurrentDirectoryA",
        .patch = my_SetCurrentDirectoryA,
        .link = (void **) &real_SetCurrentDirectoryA,
    },
};

void ddrhook1_get_launcher_path_parts(char **output_path, char **output_foldername) {
    char module_path[MAX_PATH];
    char launcher_path[MAX_PATH];

    memset(launcher_path, 0, MAX_PATH);

    if (output_path != NULL)
        *output_path = NULL;

    if (output_foldername != NULL)
        *output_foldername = NULL;

    if (GetModuleFileNameA(NULL, module_path, sizeof(module_path)) == 0)
        return;

    char *filename_ptr = NULL;
    if (GetFullPathNameA(module_path, sizeof(launcher_path), launcher_path, &filename_ptr) == 0)
        return;

    if (filename_ptr != NULL)
        launcher_path[strlen(launcher_path) - strlen(filename_ptr) - 1] = 0;

    // Trim tailing slashes
    for (int i = strlen(launcher_path) - 1; i > 0; i--) {
        if (launcher_path[i] != '\\' && launcher_path[i] != '/')
            break;

        launcher_path[i] = 0;
    }

    int idx_folder = 0;
    for (idx_folder = strlen(launcher_path); idx_folder - 1 > 0; idx_folder--) {
        if (launcher_path[idx_folder - 1] == '\\' || launcher_path[idx_folder - 1] == '/') {
            break;
        }
    }

    if (output_foldername != NULL) {
        int len = strlen(launcher_path) - idx_folder;

        if (len < 0)
            len = 0;

        *output_foldername = (char*)xmalloc(len + 1);
        memset(*output_foldername, 0, len + 1);
        strncpy(*output_foldername, launcher_path + idx_folder, len);
    }

    if (output_path != NULL) {
        size_t len = idx_folder - 1 < 0 ? 0 : idx_folder - 1;

        if (len > strlen(launcher_path))
            len = strlen(launcher_path);

        *output_path = (char*)xmalloc(len + 1);
        memset(*output_path, 0, len + 1);
        strncpy(*output_path, launcher_path, len);
    }
}

static BOOL STDCALL my_SetCurrentDirectoryA(LPCTSTR lpPathName)
{
    if (stricmp(lpPathName, "D:/HDX") == 0
    || stricmp(lpPathName, "D:\\HDX") == 0) {
        char *new_path;
        ddrhook1_get_launcher_path_parts(&new_path, NULL);

        if (new_path != NULL) {
            bool r = real_SetCurrentDirectoryA(new_path);
            log_misc("Changed directory %s -> %s", lpPathName, new_path);
            free(new_path);
            return r;
        }
    }

    return real_SetCurrentDirectoryA(lpPathName);
}

void ddrhook1_filesystem_hook_init()
{
    hook_table_apply(
        NULL, "kernel32.dll", filesystem_hook_syms, lengthof(filesystem_hook_syms));

    log_info("Inserted filesystem hooks");
}