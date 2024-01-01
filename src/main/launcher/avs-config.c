#define LOG_MODULE "avs-config"

#include <windows.h>

#include "imports/avs.h"

#include "imports/avs.h"

#include "launcher/avs-config.h"
#include "launcher/property-util.h"

#include "util/log.h"
#include "util/str.h"

#define AVS_CONFIG_ROOT_NODE "/config"

struct property* avs_config_load_from_file_path(const char *filepath)
{
    struct property *property;

    log_assert(filepath);

    log_misc("Loading avs-config from file path: %s", filepath);

    property = property_util_load_file(filepath);

    // Check if root node exists, call already errors if not
    avs_config_resolve_root_node(property);

    return property;
}

struct property_node* avs_config_resolve_root_node(struct property *property)
{
    struct property_node *node;

    log_assert(property);

    node = property_search(property, 0, AVS_CONFIG_ROOT_NODE);

    if (node == NULL) {
        log_fatal("Root node " AVS_CONFIG_ROOT_NODE " in AVS config missing");
    }

    return node;
}

void avs_config_set_mode_product(struct property *property, bool enable)
{
    struct property_node *node;

    log_assert(property);

    node = avs_config_resolve_root_node(property);

    property_remove(NULL, node, "mode/product");

#if AVS_VERSION <= 1306
    property_node_create(
        NULL, node, PROPERTY_TYPE_U8, "mode/product", enable ? 1 : 0);
#else
    property_node_create(
        NULL, node, PROPERTY_TYPE_BOOL, "mode/product", enable ? 1 : 0);
#endif
}

void avs_config_set_net_raw(struct property *property, bool enable)
{
    struct property_node *node;

    log_assert(property);

    node = avs_config_resolve_root_node(property);

    property_remove(NULL, node, "net/enable_raw");

#if AVS_VERSION <= 1306
    property_node_create(
        NULL, node, PROPERTY_TYPE_U8, "net/enable_raw", enable ? 1 : 0);
#else
    property_node_create(
        NULL, node, PROPERTY_TYPE_BOOL, "net/enable_raw", enable ? 1 : 0);
#endif
}

void avs_config_set_net_eaudp(struct property *property, bool enable)
{
    struct property_node *node;

    log_assert(property);

    node = avs_config_resolve_root_node(property);

    property_remove(NULL, node, "net/eaudp/enable");

#if AVS_VERSION <= 1306
    property_node_create(
        NULL, node, PROPERTY_TYPE_U8, "net/eaudp/enable", enable ? 1 : 0);
#else
    property_node_create(
        NULL, node, PROPERTY_TYPE_BOOL, "net/eaudp/enable", enable ? 1 : 0);
#endif
}

void avs_config_set_sntp_ea(struct property *property, bool on)
{
    struct property_node *node;

    log_assert(property);

    node = avs_config_resolve_root_node(property);

    property_remove(NULL, node, "sntp/ea_on");

#if AVS_VERSION <= 1306
    property_node_create(
        NULL, node, PROPERTY_TYPE_U8, "sntp/ea_on", on ? 1 : 0);
#else
    property_node_create(
        NULL, node, PROPERTY_TYPE_BOOL, "sntp/ea_on", on ? 1 : 0);
#endif
}

void avs_config_set_fs_root_device(struct property *property, const char *path)
{
    struct property_node *node;

    log_assert(property);
    log_assert(path);

    node = avs_config_resolve_root_node(property);

    property_remove(NULL, node, "fs/root/device");

    property_node_create(
        NULL,
        node,
        PROPERTY_TYPE_STR,
        "fs/root/device",
        path);
}

void avs_config_set_logging(struct property *property, const struct bootstrap_config *config)
{
    struct property_node *node;

    log_assert(property);
    log_assert(config);

    if (config->log_node) {
        node = avs_config_resolve_root_node(property);
        
        property_remove(NULL, node, "log");
        property_node_clone(NULL, node, config->log_node, TRUE);
    }
}

void avs_config_set_log_level(
        struct property *property,
        enum log_level loglevel)
{
    struct property_node *log_level_node;
    enum property_type type;
    const char *loglevel_str;

    log_assert(property);

    log_level_node = property_search(property, NULL, "config/log/level");

    if (!log_level_node) {
        log_fatal("config/log/level missing in AVS configuration");
    }

    type = property_node_type(log_level_node);

    // Different AVS config formats depending on AVS version, detect based on the existing values
    switch (type) {
        case PROPERTY_TYPE_STR:
            switch (loglevel) {
                case LOG_LEVEL_FATAL:
                    loglevel_str = "fatal";
                    break;

                case LOG_LEVEL_WARNING:
                    loglevel_str = "warn";
                    break;

                case LOG_LEVEL_INFO:
                    loglevel_str = "info";
                    break;

                case LOG_LEVEL_MISC:
                    loglevel_str = "misc";
                    break;

                default:
                    log_fatal("Unsupported log level: %d", loglevel);
                    break;
            }

            property_node_remove(log_level_node);
            property_node_create(property, log_level_node, PROPERTY_TYPE_STR, NULL, loglevel_str);

            break;

        case PROPERTY_TYPE_U32:
            property_node_remove(log_level_node);
            property_node_create(property, log_level_node, PROPERTY_TYPE_U32, NULL, loglevel);

            break;

        default:
            log_fatal("Unsupported property type %d for config/log/level node in AVS config", type);
            break;
    }
}

void avs_config_set_local_fs_path_dev_nvram_and_raw(
        struct property* property,
        const char* dev_nvram_raw_path)
{
    char path_dev_raw[MAX_PATH];
    char path_dev_nvram[MAX_PATH];

    struct property_node *fs_node;
    struct property_node *mounttable_node;
    struct property_node *vfs_node;

    log_assert(property);
    log_assert(dev_nvram_raw_path);

    str_cpy(path_dev_raw, sizeof(path_dev_raw), dev_nvram_raw_path);
    str_cat(path_dev_raw, sizeof(path_dev_raw), "/dev/raw");

    str_cpy(path_dev_nvram, sizeof(path_dev_nvram), dev_nvram_raw_path);
    str_cat(path_dev_nvram, sizeof(path_dev_nvram), "/dev/nvram");

    fs_node = property_search(property, NULL, "config/fs");

    if (!fs_node) {
        log_fatal("Cannot find config/fs in avs config");
    }

    // Check if "new" mounttable config is used for dev/nvram and dev/raw or legacy config
    if (property_search(property, fs_node, "mounttable")) {
        property_remove(property, fs_node, "mounttable");

        mounttable_node = property_node_create(property, fs_node, PROPERTY_TYPE_VOID, "mounttable");

        vfs_node = property_node_create(property, mounttable_node, PROPERTY_TYPE_VOID, "vfs");

        property_node_create(property, vfs_node, PROPERTY_TYPE_ATTR, "name", "boot");
        property_node_create(property, vfs_node, PROPERTY_TYPE_ATTR, "fstype", "fs");
        property_node_create(property, vfs_node, PROPERTY_TYPE_ATTR, "src", path_dev_raw);
        property_node_create(property, vfs_node, PROPERTY_TYPE_ATTR, "dest", "/dev/raw");
        property_node_create(property, vfs_node, PROPERTY_TYPE_ATTR, "opt", "vf=1,posix=1");

        vfs_node = property_node_create(property, mounttable_node, PROPERTY_TYPE_VOID, "vfs");

        property_node_create(property, vfs_node, PROPERTY_TYPE_ATTR, "name", "boot");
        property_node_create(property, vfs_node, PROPERTY_TYPE_ATTR, "fstype", "fs");
        property_node_create(property, vfs_node, PROPERTY_TYPE_ATTR, "src", path_dev_nvram);
        property_node_create(property, vfs_node, PROPERTY_TYPE_ATTR, "dest", "/dev/nvram");
        property_node_create(property, vfs_node, PROPERTY_TYPE_ATTR, "opt", "vf=1,posix=1");
    } else {
        property_util_node_replace_str(property, fs_node, "nvram/device", path_dev_raw);
        property_util_node_replace_str(property, fs_node, "nvram/fstype", "fs");
        property_util_node_replace_str(property, fs_node, "nvram/option", "vf=1,posix=1");

        property_util_node_replace_str(property, fs_node, "raw/device", path_dev_nvram);
        property_util_node_replace_str(property, fs_node, "raw/fstype", "fs");
        property_util_node_replace_str(property, fs_node, "raw/option", "vf=1,posix=1");
    }    
}