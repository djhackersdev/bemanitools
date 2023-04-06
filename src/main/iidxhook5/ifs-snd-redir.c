#define LOG_MODULE "ifs-snd-redir"

#include <stdint.h>
#include <string.h>

#include "hook/table.h"

#include "imports/avs.h"

#include "iidxhook5/ifs-snd-redir.h"

#include "util/log.h"
#include "util/str.h"

static void *(*real_avs_fs_open)(const char *path, int mode, int flags);
static void *my_avs_fs_open(const char *path, int mode, int flags);

static const struct hook_symbol iidxhook5_ifs_snd_redir_hook_syms[] = {
    {.name = "XC058ba50000b6", // avs_fs_open
     .patch = my_avs_fs_open,
     .link = (void **) &real_avs_fs_open},
};

static void *my_avs_fs_open(const char *path, int mode, int flags)
{
    void *handle;
    char redir_path[MAX_PATH];

    // Trap virtual path /sd00/*, /sd01/*, /sd02/*, /sd03/* that contain
    // sound data that is stored in ifs files in data/imagefs
    // Create a generice override strategy by redirecting to local folder
    // /sound/*. Check if open succeeds and return that. Otherwise, stay on
    // imagefs path to stay compatible with stock data.

    if (!strncmp(path, "/sd0", 4)) {
        strcpy(redir_path, "/data/sound");
        strcat(redir_path, &path[5]);

        log_misc("Sound data redirect: %s -> %s", path, redir_path);

        handle = real_avs_fs_open(redir_path, mode, flags);

        if (handle != NULL) {
            log_misc("Success, use %s", redir_path);
            return handle;
        } else {
            log_misc("Failure, use %s", path);
            return real_avs_fs_open(path, mode, flags);
        }
    }

    return real_avs_fs_open(path, mode, flags);
}

void iidxhook5_ifs_snd_redir_init()
{
    hook_table_apply(
        NULL,
        "libavs-win32.dll",
        iidxhook5_ifs_snd_redir_hook_syms,
        lengthof(iidxhook5_ifs_snd_redir_hook_syms));

    log_info("Inserted ifs-snd-redir hooks");
}
