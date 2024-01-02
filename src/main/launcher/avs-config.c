#define LOG_MODULE "avs-config"

#include <windows.h>

#include "avs-util/error.h"

#include "imports/avs.h"

#include "launcher/avs-config.h"
#include "launcher/property-util.h"

#include "util/log.h"
#include "util/str.h"

#define AVS_CONFIG_ROOT_NODE "/config"

struct property* avs_config_load(const char *filepath)
{
    struct property *property;

    log_assert(filepath);

    log_info("Loading from file path: %s", filepath);

    property = property_util_load(filepath);

    // Check if root node exists, call already errors if not
    avs_config_root_get(property);

    return property;
}

struct property_node* avs_config_root_get(struct property *property)
{
    struct property_node *node;

    log_assert(property);

    node = property_search(property, 0, AVS_CONFIG_ROOT_NODE);

    if (node == NULL) {
        log_fatal("Root node " AVS_CONFIG_ROOT_NODE " in AVS config missing");
    }

    return node;
}

void avs_config_fs_root_device_get(struct property_node *node, char* buffer, size_t size)
{
    struct property_node *device_node;
    avs_error error;
    
    log_assert(node);
 
    device_node = property_search(NULL, node, "fs/root/device");

    if (device_node == NULL) {
        log_fatal("Could not find node fs/root/device AVS config");
    }

    error = property_node_read(device_node, PROPERTY_TYPE_STR, buffer, size);

    if (AVS_IS_ERROR(error)) {
        log_fatal("fs/root/device, property read failed: %s", avs_util_error_str(error));
    }
}

void avs_config_mode_product_set(struct property_node *node, bool enable)
{
    log_assert(node);

#if AVS_VERSION <= 1306
    property_util_node_u8_replace(NULL, node, "mode/product", enable ? 1 : 0);
#else
    property_util_node_bool_replace(NULL, node, "mode/product", enable);
#endif
}

void avs_config_net_raw_set(struct property_node *node, bool enable)
{
    log_assert(node);

#if AVS_VERSION <= 1306
    property_util_node_u8_replace(NULL, node, "net/enable_raw", enable ? 1 : 0);
#else
    property_util_node_bool_replace(NULL, node, "net/enable_raw", enable);
#endif
}

void avs_config_net_eaudp_set(struct property_node *node, bool enable)
{
    log_assert(node);

#if AVS_VERSION <= 1306
    property_util_node_u8_replace(NULL, node, "net/eaudp/enable", enable ? 1 : 0);
#else
    property_util_node_bool_replace(NULL, node, "net/eaudp/enable", enable);
#endif
}

void avs_config_sntp_ea_set(struct property_node *node, bool on)
{
    log_assert(node);

#if AVS_VERSION <= 1306
    property_util_node_u8_replace(NULL, node, "sntp/ea_on", on ? 1 : 0);
#else
    property_util_node_bool_replace(NULL, node, "sntp/ea_on", on);
#endif
}

void avs_config_log_level_set(struct property_node *node, const char *level)
{
    log_assert(node);
    log_assert(level);

#if AVS_VERSION <= 1306
    uint32_t level_value;

    if (str_eq(level, "fatal")) {
        level_value = 1;
    } else if (str_eq(level, "warning")) {
        level_value = 2;
    } else if (str_eq(level, "info")) {
        level_value = 3;
    } else if (str_eq(level, "misc")) {
        level_value = 4;
    } else if (str_eq(level, "all")) {
        level_value = 4;
    } else if (str_eq(level, "disable")) {
        level_value = 0;
    } else if (str_eq(level, "default")) {
        level_value = 4;
    } else {
        log_fatal("Unknown log level string %s", level);
    }

    property_util_node_u32_replace(NULL, node, "log/level", level_value);
#else
    property_util_node_str_replace(NULL, node, "log/level", level);
#endif
}

void avs_config_log_name_set(struct property_node *node, const char *name)
{
    log_assert(node);
    log_assert(name);

    property_util_node_str_replace(NULL, node, "log/name", name);
}

void avs_config_log_file_set(struct property_node *node, const char *file)
{
    log_assert(node);
    log_assert(file);

    property_util_node_str_replace(NULL, node, "log/file", file);
}

void avs_config_log_buffer_size_set(struct property_node *node, uint32_t size)
{
    log_assert(node);

    property_util_node_u32_replace(NULL, node, "log/sz_buf", size);
}

void avs_config_log_output_delay_set(struct property_node *node, uint16_t delay_ms)
{
    log_assert(node);

    property_util_node_u16_replace(NULL, node, "log/output_delay", delay_ms);
}

void avs_config_log_enable_console_set(struct property_node *node, bool enable)
{
    log_assert(node);

#if AVS_VERSION <= 1306
    property_util_node_u8_replace(NULL, node, "log/enable_console", enable ? 1 : 0);
#else
    property_util_node_bool_replace(NULL, node, "log/enable_console", enable);
#endif
}

void avs_config_log_enable_sci_set(struct property_node *node, bool enable)
{
    log_assert(node);

#if AVS_VERSION <= 1306
    property_util_node_u8_replace(NULL, node, "log/enable_netsci", enable ? 1 : 0);
#else
    property_util_node_bool_replace(NULL, node, "log/enable_netsci", enable);
#endif
}

void avs_config_log_enable_net_set(struct property_node *node, bool enable)
{
    log_assert(node);

#if AVS_VERSION <= 1306
    property_util_node_u8_replace(NULL, node, "log/enable_netlog", enable ? 1 : 0);
#else
    property_util_node_bool_replace(NULL, node, "log/enable_netlog", enable);
#endif
}

void avs_config_log_enable_file_set(struct property_node *node, bool enable)
{
    log_assert(node);

#if AVS_VERSION <= 1306
    property_util_node_u8_replace(NULL, node, "log/enable_file", enable ? 1 : 0);
#else
    property_util_node_bool_replace(NULL, node, "log/enable_file", enable);
#endif
}

void avs_config_log_rotate_set(struct property_node *node, bool rotate)
{
    log_assert(node);

#if AVS_VERSION <= 1306
    property_util_node_u8_replace(NULL, node, "log/rotate", rotate ? 1 : 0);
#else
    property_util_node_bool_replace(NULL, node, "log/rotate", rotate);
#endif
}

void avs_config_log_append_set(struct property_node *node, bool append)
{
    log_assert(node);

#if AVS_VERSION <= 1306
    property_util_node_u8_replace(NULL, node, "log/append", append ? 1 : 0);
#else
    property_util_node_bool_replace(NULL, node, "log/append", append);
#endif
}

void avs_config_log_count_set(struct property_node *node, uint16_t count)
{
    log_assert(node);

    property_util_node_u16_replace(NULL, node, "log/gen", count);
}

void avs_config_set_log_level(
        struct property_node *node,
        enum log_level loglevel)
{
    const char *str;

    log_assert(node);

    switch (loglevel) {
        case LOG_LEVEL_FATAL:
            str = "fatal";
            break;

        case LOG_LEVEL_WARNING:
            str = "warn";
            break;

        case LOG_LEVEL_INFO:
            str = "info";
            break;

        case LOG_LEVEL_MISC:
            str = "misc";
            break;

        default:
            log_fatal("Unsupported log level: %d", loglevel);
            break;
    }

    avs_config_log_level_set(node, str);
}

void avs_config_local_fs_path_dev_nvram_and_raw_set(
        struct property_node* node,
        const char* dev_nvram_raw_path)
{
    char path_dev_raw[MAX_PATH];
    char path_dev_nvram[MAX_PATH];

    struct property_node *fs_node;
    struct property_node *mounttable_node;
    struct property_node *vfs_node;

    log_assert(node);
    log_assert(dev_nvram_raw_path);

    str_cpy(path_dev_raw, sizeof(path_dev_raw), dev_nvram_raw_path);
    str_cat(path_dev_raw, sizeof(path_dev_raw), "/dev/raw");

    str_cpy(path_dev_nvram, sizeof(path_dev_nvram), dev_nvram_raw_path);
    str_cat(path_dev_nvram, sizeof(path_dev_nvram), "/dev/nvram");

    fs_node = property_search(NULL, node, "fs");

    if (!fs_node) {
        log_fatal("Cannot find 'fs' node in avs config");
    }

    // Check if "new" mounttable config is used for dev/nvram and dev/raw or legacy config
    if (property_search(NULL, fs_node, "mounttable")) {
        property_remove(NULL, fs_node, "mounttable");

        mounttable_node = property_node_create(NULL, fs_node, PROPERTY_TYPE_VOID, "mounttable");

        vfs_node = property_node_create(NULL, mounttable_node, PROPERTY_TYPE_VOID, "vfs");

        property_node_create(NULL, vfs_node, PROPERTY_TYPE_ATTR, "name", "boot");
        property_node_create(NULL, vfs_node, PROPERTY_TYPE_ATTR, "fstype", "fs");
        property_node_create(NULL, vfs_node, PROPERTY_TYPE_ATTR, "src", path_dev_raw);
        property_node_create(NULL, vfs_node, PROPERTY_TYPE_ATTR, "dest", "/dev/raw");
        property_node_create(NULL, vfs_node, PROPERTY_TYPE_ATTR, "opt", "vf=1,posix=1");

        vfs_node = property_node_create(NULL, mounttable_node, PROPERTY_TYPE_VOID, "vfs");

        property_node_create(NULL, vfs_node, PROPERTY_TYPE_ATTR, "name", "boot");
        property_node_create(NULL, vfs_node, PROPERTY_TYPE_ATTR, "fstype", "fs");
        property_node_create(NULL, vfs_node, PROPERTY_TYPE_ATTR, "src", path_dev_nvram);
        property_node_create(NULL, vfs_node, PROPERTY_TYPE_ATTR, "dest", "/dev/nvram");
        property_node_create(NULL, vfs_node, PROPERTY_TYPE_ATTR, "opt", "vf=1,posix=1");
    } else {
        property_util_node_str_replace(NULL, fs_node, "nvram/device", path_dev_raw);
        property_util_node_str_replace(NULL, fs_node, "nvram/fstype", "fs");
        property_util_node_str_replace(NULL, fs_node, "nvram/option", "vf=1,posix=1");

        property_util_node_str_replace(NULL, fs_node, "raw/device", path_dev_nvram);
        property_util_node_str_replace(NULL, fs_node, "raw/fstype", "fs");
        property_util_node_str_replace(NULL, fs_node, "raw/option", "vf=1,posix=1");
    }    
}