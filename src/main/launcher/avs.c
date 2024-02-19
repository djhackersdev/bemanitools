#define LOG_MODULE "avs"

#include <windows.h>

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "core/log-bt.h"
#include "core/log.h"

#include "imports/avs.h"

#include "launcher/avs-config.h"
#include "launcher/avs.h"
#include "launcher/property-util.h"

#include "util/codepage.h"
#include "util/fs.h"
#include "util/mem.h"
#include "util/str.h"

#if AVS_VERSION < 1600
#define AVS_HAS_STD_HEAP
#endif

static void *avs_heap;

#ifdef AVS_HAS_STD_HEAP
static void *std_heap;
#endif

/* Gratuitous API changes orz */
static AVS_LOG_WRITER(_avs_context_log_writer, chars, nchars, ctx)
{
    wchar_t *utf16;
    char *utf8;
    int utf16_len;
    int utf8_len;
    int result;

    /* Ignore existing NUL terminator */

    nchars--;

    /* Transcode shit_jis to UTF-8 */

    utf16_len = MultiByteToWideChar(CP_SHIFT_JIS, 0, chars, nchars, NULL, 0);

    if (utf16_len == 0) {
        abort();
    }

    utf16 = xmalloc(sizeof(*utf16) * utf16_len);
    result =
        MultiByteToWideChar(CP_SHIFT_JIS, 0, chars, nchars, utf16, utf16_len);

    if (result == 0) {
        abort();
    }

    utf8_len =
        WideCharToMultiByte(CP_UTF8, 0, utf16, utf16_len, NULL, 0, NULL, NULL);

    if (utf8_len == 0) {
        abort();
    }

    utf8 = xmalloc(utf8_len + 3);
    result = WideCharToMultiByte(
        CP_UTF8, 0, utf16, utf16_len, utf8, utf8_len, NULL, NULL);

    if (result == 0) {
        abort();
    }

#if AVS_VERSION >= 1500
    utf8[utf8_len + 0] = '\r';
    utf8[utf8_len + 1] = '\n';

    utf8_len += 2;
#endif

    // Clean string terminate
    utf8[utf8_len] = '\0';

    // Write to launcher's dedicated logging backend
    core_log_bt_direct_sink_write(utf8, utf8_len);

    /* Clean up */

    free(utf8);
    free(utf16);
}

static void _avs_switch_log_engine()
{
    // Switch the logging backend now that AVS is booted to use a single logging
    // engine which avoids concurrency issues as AVS runs it's own async logger
    // thread
    core_log_impl_set(
        log_body_misc, log_body_info, log_body_warning, log_body_fatal);

    log_misc("Switched logging engine to AVS");
}

void avs_fs_assert_root_device_exists(struct property_node *node)
{
    char root_device_path[PATH_MAX];
    char cwd_path[PATH_MAX];

    avs_config_fs_root_device_get(
        node, root_device_path, sizeof(root_device_path));
    getcwd(cwd_path, sizeof(cwd_path));

    if (!path_exists(root_device_path)) {
        log_fatal(
            "Root device path '%s' does not exist in current working dir '%s'",
            root_device_path,
            cwd_path);
    }
}

void avs_fs_mountpoint_dir_create(
    struct property_node *node, const char *folder_name)
{
    char fs_path[1024];
    char fs_type[255];
    char device_path[1024];
    struct property_node *fs_node;
    int res;

    memset(fs_path, 0, sizeof(fs_path));
    memset(fs_type, 0, sizeof(fs_type));

    str_cpy(fs_path, sizeof(fs_path), "/fs/");
    str_cat(fs_path, sizeof(fs_path), folder_name);

    fs_node = property_search(NULL, node, fs_path);

    if (!fs_node) {
        log_warning(
            "Could not find file system node %s in avs configuration", fs_path);
        return;
    }

    res = property_node_refer(
        NULL,
        fs_node,
        "device",
        PROPERTY_TYPE_STR,
        device_path,
        sizeof(device_path));

    if (res < 0) {
        log_fatal(
            "Getting 'device' attribute from avs config entry %s failed",
            fs_path);
    }

    // 'fstype' attribute is optional and defaults to value 'fs'
    if (!property_search(NULL, fs_node, "fstype")) {
        if (path_exists(device_path)) {
            // skip if exists already
            return;
        }

        log_misc("Creating avs directory %s", device_path);

        if (!path_mkdir(device_path)) {
            log_fatal("Creating directory %s failed", device_path);
        }
    } else {
        res = property_node_refer(
            NULL,
            fs_node,
            "fstype",
            PROPERTY_TYPE_STR,
            fs_type,
            sizeof(fs_type));

        if (res < 0) {
            log_fatal(
                "Getting 'fstype' attribute from avs config entry %s failed",
                fs_path);
        }

        if (!strcmp(fs_type, "fs") || !strcmp(fs_type, "nvram")) {
            if (path_exists(device_path)) {
                // skip if exists already
                return;
            }

            log_misc("Creating avs directory %s", device_path);

            if (!path_mkdir(device_path)) {
                log_fatal("Creating directory %s failed", device_path);
            }
        } else {
            log_fatal(
                "Cannot create folders for unsupported file system type %s of "
                "path %s in avs config",
                fs_type,
                fs_path);
        }
    }
}

void avs_init(
    struct property_node *node, uint32_t avs_heap_size, uint32_t std_heap_size)
{
    log_assert(node);
    log_assert(avs_heap_size > 0);
    log_assert(std_heap_size > 0);

    log_info("init");

    avs_heap = VirtualAlloc(
        NULL, avs_heap_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (avs_heap == NULL) {
        log_fatal(
            "Failed to VirtualAlloc %d byte AVS heap: %08x",
            avs_heap_size,
            (unsigned int) GetLastError());
    }

#ifdef AVS_HAS_STD_HEAP
    std_heap = VirtualAlloc(
        NULL, std_heap_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (std_heap == NULL) {
        log_fatal(
            "Failed to VirtualAlloc %d byte \"std\" heap: %08x",
            std_heap_size,
            (unsigned int) GetLastError());
    }
#endif

    log_info("Calling avs_boot");

#ifdef AVS_HAS_STD_HEAP
    avs_boot(
        node,
        std_heap,
        std_heap_size,
        avs_heap,
        avs_heap_size,
        _avs_context_log_writer,
        NULL);
#else
    /* AVS v2.16.xx and I suppose onward uses a unified heap */
    avs_boot(
        node, avs_heap, avs_heap_size, NULL, _avs_context_log_writer, NULL);
#endif

    _avs_switch_log_engine();

    log_misc("init done");
}

void avs_fs_file_copy(const char *src, const char *dst)
{
    struct avs_stat st;

    log_assert(src);
    log_assert(dst);

    log_misc("Copying %s to %s...", src, dst);

    if (!avs_fs_lstat(src, &st)) {
        log_fatal("File source %s does not exist or is not accessible", src);
    }

    if (avs_fs_copy(src, dst) < 0) {
        log_fatal("Failed copying file %s to %s", src, dst);
    }
}

void avs_fs_dir_log(const char *path)
{
    const char *name;

    log_assert(path);

    avs_desc dir = avs_fs_opendir(path);

    if (dir < 0) {
        log_warning(
            "Opening avs dir %s failed, skipping logging contents", path);
    }

    log_misc("Contents of %s:", path);

    do {
        name = avs_fs_readdir(dir);

        if (name == NULL) {
            break;
        }

        log_misc("%s", name);
    } while (name != NULL);

    avs_fs_closedir(dir);
}

void avs_fini(void)
{
    log_info("fini");

    avs_shutdown();

#ifdef AVS_HAS_STD_HEAP
    VirtualFree(std_heap, 0, MEM_RELEASE);
#endif

    VirtualFree(avs_heap, 0, MEM_RELEASE);

    log_misc("fini done");
}
