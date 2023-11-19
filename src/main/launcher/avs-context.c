#include <windows.h>

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "imports/avs.h"

#include "launcher/avs-context.h"

#include "util/fs.h"
#include "util/log.h"
#include "util/str.h"

static void *avs_heap;

#ifdef AVS_HAS_STD_HEAP
static void *std_heap;
#endif

static void _avs_context_create_config_fs_dir(
        struct property *prop,
        struct property_node *node,
        const char *folder_name)
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

    fs_node = property_search(prop, node, fs_path);

    if (!fs_node) {
        log_warning("Could not find file system node %s in avs configuration", fs_path);
        return;
    }

    res = property_node_refer(prop, fs_node, "device", PROPERTY_TYPE_STR,
        device_path, sizeof(device_path));

    if (res < 0) {
        log_fatal("Getting 'device' attribute from avs config entry %s failed", fs_path);
    }

    // 'fstype' attribute is optional and defaults to value 'fs'
    if (!property_search(prop, fs_node, "fstype")) {
        if (path_exists(device_path)) {
            // skip if exists already
            return;
        }

        log_misc("Creating avs directory %s", device_path);

        if (!path_mkdir(device_path)) {
            log_fatal("Creating directory %s failed", device_path);
        }
    } else {
        res = property_node_refer(prop, fs_node, "fstype", PROPERTY_TYPE_STR,
                fs_type, sizeof(fs_type));

        if (res < 0) {
            log_fatal("Getting 'fstype' attribute from avs config entry %s failed", fs_path);
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
            log_fatal("Cannot create folders for unsupported file system type %s of path %s in avs config",
                fs_type, fs_path);
        }
    }
}

void avs_context_init(
    struct property *config_prop,
    struct property_node *config_node,
    uint32_t avs_heap_size,
    uint32_t std_heap_size,
    avs_log_writer_t log_writer,
    void *log_writer_ctx)
{
    log_misc("Creating AVS file system directories for nvram and raw if not exist...");

    // create nvram and raw directories if possible for non-mounttable configurations
    _avs_context_create_config_fs_dir(config_prop, config_node, "nvram");
    _avs_context_create_config_fs_dir(config_prop, config_node, "raw");

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

#ifdef AVS_HAS_STD_HEAP
    avs_boot(
        config_node,
        std_heap,
        std_heap_size,
        avs_heap,
        avs_heap_size,
        log_writer,
        log_writer_ctx);
#else
    /* AVS v2.16.xx and I suppose onward uses a unified heap */
    avs_boot(config_node, avs_heap, avs_heap_size, NULL, log_writer, log_writer_ctx);
#endif
}

void avs_context_fini(void)
{
    avs_shutdown();

#ifdef AVS_HAS_STD_HEAP
    VirtualFree(std_heap, 0, MEM_RELEASE);
#endif

    VirtualFree(avs_heap, 0, MEM_RELEASE);
}
