#define LOG_MODULE "avs-ext-fs"

#include "api/core/log.h"

#include "iface-core/log.h"

#include "imports/avs.h"

void avs_ext_fs_dir_list_log(
    const char *path, bt_core_log_message_t log_message)
{
    avs_desc desc;
    const char *dirent_name;

    desc = avs_fs_opendir(path);

    if (!desc) {
        log_warning("Opening directory '%s' failed", path);
        return;
    }

    log_message(LOG_MODULE, "Directory contents of: %s", path);

    dirent_name = avs_fs_readdir(desc);

    while (dirent_name != NULL) {
        log_message(LOG_MODULE, "  %s", dirent_name);

        dirent_name = avs_fs_readdir(desc);
    }

    avs_fs_closedir(desc);
}