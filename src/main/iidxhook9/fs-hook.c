#define LOG_MODULE "fs-hook"

#include <stdint.h>
#include <string.h>

#include "hook/table.h"

#include "imports/avs.h"

#include "iidxhook9/fs-hook.h"

#include "util/log.h"
#include "util/str.h"

static void *(*real_avs_fs_mount)(const char *dest, const char *src, const char *fs_type, const char *options);
static void *my_avs_fs_mount(const char *dest, const char *src, const char *fs_type, const char *options);

static const struct hook_symbol avs_fs_hook_syms[] = {
    {.name = "XCgsqzn000004b", // avs_fs_mount
     .ordinal = 76,
     .patch = my_avs_fs_mount,
     .link = (void **) &real_avs_fs_mount},
};

static void *my_avs_fs_mount(const char *dest, const char *src, const char *fs_type, const char *options)
{
    // quick check for "F:\"
    if (src[0] == 'F' && src[1] == ':' && src[2] == '\0') {
        const char* dev_folder_drive = "dev/vfs/drive_f/";
        log_misc("Redirecting %s to %s", src, dev_folder_drive);

        CreateDirectoryA("dev/vfs/", NULL);
        CreateDirectoryA("dev/vfs/drive_f/", NULL);

        return real_avs_fs_mount(dest, dev_folder_drive, fs_type, options);
    }
    if (src[0] == 'e' && src[1] == ':' && src[2] == '/' && src[3] == '\0') {
        const char* dev_folder_drive = "dev/vfs/drive_e/";
        log_misc("Redirecting %s to %s", src, dev_folder_drive);

        CreateDirectoryA("dev/vfs/", NULL);
        CreateDirectoryA("dev/vfs/drive_e/", NULL);

        return real_avs_fs_mount(dest, dev_folder_drive, fs_type, options);
    }

    return real_avs_fs_mount(dest, src, fs_type, options);
}

void iidxhook9_fs_hooks_init()
{
    hook_table_apply(
        NULL,
        "avs2-core.dll",
        avs_fs_hook_syms,
        lengthof(avs_fs_hook_syms));

    log_info("Inserted avs fs hooks");
}
