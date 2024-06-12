#define LOG_MODULE "avs-config"

#include <windows.h>

#include "avs-ext/error.h"

#include "iface-core/log.h"

#include "core/property-node-ext.h"
#include "core/property-node.h"
#include "core/property.h"

#include "launcher/avs-config.h"

#include "util/str.h"

#define AVS_CONFIG_ROOT_NODE "/config"

static const char *_avs_config_property_mounttable_path =
    "/config/fs/mounttable";

static void _avs_config_node_vfs_copy(
    core_property_node_t *parent, const core_property_node_t *source)
{
    // Use max path size to fit dst and src fs paths
    char data[MAX_PATH];

    // Remark: Using property_node_clone doesn't work here
    // Cloning non-deep only clones the vfs node. Cloning deep doesn't seem
    // to work with arbitrary attributes that don't follow the general
    // design of a property structure. This seems to require clear typing for
    // nodes in order to allow property_node_clone to work

    // Ignore errors and default to empty
    memset(data, 0, sizeof(data));
    core_property_node_ext_attr_read(source, "name@", data, sizeof(data));
    core_property_node_ext_attr_replace(parent, "name@", data);

    memset(data, 0, sizeof(data));
    core_property_node_ext_attr_read(source, "fstype@", data, sizeof(data));
    core_property_node_ext_attr_replace(parent, "fstype@", data);

    memset(data, 0, sizeof(data));
    core_property_node_ext_attr_read(source, "src@", data, sizeof(data));
    core_property_node_ext_attr_replace(parent, "src@", data);

    memset(data, 0, sizeof(data));
    core_property_node_ext_attr_read(source, "dst@", data, sizeof(data));
    core_property_node_ext_attr_replace(parent, "dst@", data);

    memset(data, 0, sizeof(data));
    core_property_node_ext_attr_read(source, "opt@", data, sizeof(data));
    core_property_node_ext_attr_replace(parent, "opt@", data);
}

static core_property_node_result_t
_avs_config_mounttable_vfs_nodes_merge_strategy_do(
    core_property_node_t *parent,
    const core_property_node_t *source,
    void *ctx,
    bool *consumed,
    core_property_node_ext_merge_recursion_do_t node_merge_recursion_do)
{
    core_property_node_t parent_child;
    core_property_node_t tmp;
    core_property_node_t source_child;
    core_property_node_result_t result;

    char parent_child_name[CORE_PROPERTY_NODE_NAME_SIZE_MAX];
    char name_parent[CORE_PROPERTY_NODE_ATTR_NAME_SIZE_MAX];
    char dst_parent[CORE_PROPERTY_NODE_ATTR_NAME_SIZE_MAX];

    char source_child_name[CORE_PROPERTY_NODE_NAME_SIZE_MAX];
    char name_source[CORE_PROPERTY_NODE_ATTR_NAME_SIZE_MAX];
    char dst_source[CORE_PROPERTY_NODE_ATTR_NAME_SIZE_MAX];

    bool found_parent;

    result = core_property_node_child_get(source, &source_child);

    if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        return CORE_PROPERTY_NODE_RESULT_SUCCESS;
    }

    if (result != CORE_PROPERTY_NODE_RESULT_SUCCESS) {
        core_property_node_fatal_on_error(result);
    }

    *consumed = false;

    while (result == CORE_PROPERTY_NODE_RESULT_SUCCESS) {
        result = core_property_node_name_get(
            &source_child, source_child_name, sizeof(source_child_name));
        core_property_node_fatal_on_error(result);

        if (str_eq(source_child_name, "vfs")) {
            *consumed = true;

            result = core_property_node_child_get(parent, &parent_child);

            if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND ||
                result != CORE_PROPERTY_NODE_RESULT_SUCCESS) {
                core_property_node_fatal_on_error(result);
            }

            found_parent = false;

            while (result == CORE_PROPERTY_NODE_RESULT_SUCCESS) {
                result = core_property_node_name_get(
                    &parent_child,
                    parent_child_name,
                    sizeof(parent_child_name));
                core_property_node_fatal_on_error(result);

                if (str_eq(parent_child_name, "vfs")) {
                    result = core_property_node_ext_attr_read(
                        &source_child,
                        "name@",
                        name_source,
                        sizeof(name_source));

                    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
                        log_fatal(
                            "Missing 'name' attribute on avs config mounttable "
                            "vfs source node");
                    }

                    result = core_property_node_ext_attr_read(
                        &source_child, "dst@", dst_source, sizeof(dst_source));

                    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
                        log_fatal(
                            "Missing 'dst' attribute on avs config mounttable "
                            "vfs source node");
                    }

                    result = core_property_node_ext_attr_read(
                        &parent_child,
                        "name@",
                        name_parent,
                        sizeof(name_parent));

                    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
                        log_fatal(
                            "Missing 'name' attribute on avs config mounttable "
                            "vfs parent node");
                    }

                    result = core_property_node_ext_attr_read(
                        &parent_child, "dst@", dst_parent, sizeof(dst_parent));

                    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
                        log_fatal(
                            "Missing 'dst' attribute on avs config mounttable "
                            "vfs parent node");
                    }

                    // Found existing matching node on parent, replace it
                    if (str_eq(name_source, name_parent) &&
                        str_eq(dst_source, dst_parent)) {
                        _avs_config_node_vfs_copy(&parent_child, &source_child);

                        found_parent = true;
                        break;
                    }
                }

                result =
                    core_property_node_next_sibling_get(&parent_child, &tmp);
                memcpy(&parent_child, &tmp, sizeof(core_property_node_t));

                if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
                    core_property_node_fatal_on_error(result);
                }
            }

            // Not found an existing node that got replaced, insert/merge new
            // data
            if (!found_parent) {
                result = core_property_node_void_create(
                    parent, "vfs", &parent_child);
                core_property_node_fatal_on_error(result);

                _avs_config_node_vfs_copy(&parent_child, &source_child);
            }
        }

        result = core_property_node_next_sibling_get(&source_child, &tmp);
        memcpy(&source_child, &tmp, sizeof(core_property_node_t));

        if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
            core_property_node_fatal_on_error(result);
        }
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

core_property_t *avs_config_load(const char *filepath)
{
    core_property_t *property;
    core_property_result_t result;
    core_property_node_t node;

    log_assert(filepath);

    log_info("Loading from file path: %s", filepath);

    result = core_property_file_load(filepath, &property);
    core_property_fatal_on_error(result);

    // Check if root node exists, call already errors if not
    avs_config_root_get(property, &node);

    return property;
}

void avs_config_root_get(
    const core_property_t *property, core_property_node_t *node)
{
    core_property_node_result_t result;
    char node_name[128];

    log_assert(property);
    log_assert(node);

    result = core_property_root_node_get(property, node);
    core_property_fatal_on_error(result);

    result = core_property_node_name_get(node, node_name, sizeof(node_name));
    core_property_node_fatal_on_error(result);

    if (!str_eq(node_name, "config")) {
        log_fatal("Root node " AVS_CONFIG_ROOT_NODE " in AVS config missing");
    } else {
        core_property_node_fatal_on_error(result);
    }
}

core_property_t *avs_config_property_merge(
    const core_property_t *parent, const core_property_t *source)
{
    core_property_node_ext_merge_strategies_t strategies;
    core_property_t *merged;
    core_property_node_result_t result;

    log_assert(parent);
    log_assert(source);

    strategies.num = 2;

    strategies.entry[0].path = _avs_config_property_mounttable_path;
    strategies.entry[0].merge_strategy_do =
        _avs_config_mounttable_vfs_nodes_merge_strategy_do;

    strategies.entry[1].path = "";
    strategies.entry[1].merge_strategy_do =
        core_property_node_ext_merge_strategy_default_do;

    result = core_property_node_ext_merge_with_strategies_do(
        parent, source, &strategies, &merged);
    core_property_node_fatal_on_error(result);

    return merged;
}

void avs_config_fs_root_device_get(
    const core_property_node_t *node, char *buffer, size_t size)
{
    core_property_node_t device_node;
    core_property_node_result_t result;

    log_assert(node);
    log_assert(buffer);
    log_assert(size > 0);

    result = core_property_node_search(node, "fs/root/device", &device_node);

    if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        log_fatal("Could not find node fs/root/device AVS config");
    } else {
        core_property_node_fatal_on_error(result);
    }

    result = core_property_node_str_read(&device_node, buffer, size);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        log_fatal(
            "fs/root/device, property read failed: %s",
            core_property_node_result_to_str(result));
    }
}

void avs_config_mode_product_set(core_property_node_t *node, bool enable)
{
    core_property_node_result_t result;

    log_assert(node);

#if AVS_VERSION <= 1306
    result =
        core_property_node_ext_u8_replace(node, "mode/product", enable ? 1 : 0);
    core_property_node_fatal_on_error(result);
#else
    result = core_property_node_ext_bool_replace(node, "mode/product", enable);
    core_property_node_fatal_on_error(result);
#endif
}

void avs_config_net_raw_set(core_property_node_t *node, bool enable)
{
    core_property_node_result_t result;

    log_assert(node);

#if AVS_VERSION <= 1306
    result = core_property_node_ext_u8_replace(
        node, "net/enable_raw", enable ? 1 : 0);
    core_property_node_fatal_on_error(result);
#else
    result =
        core_property_node_ext_bool_replace(node, "net/enable_raw", enable);
    core_property_node_fatal_on_error(result);
#endif
}

void avs_config_net_eaudp_set(core_property_node_t *node, bool enable)
{
    core_property_node_result_t result;

    log_assert(node);

#if AVS_VERSION <= 1306
    result = core_property_node_ext_u8_replace(
        node, "net/eaudp/enable", enable ? 1 : 0);
    core_property_node_fatal_on_error(result);
#else
    result =
        core_property_node_ext_bool_replace(node, "net/eaudp/enable", enable);
    core_property_node_fatal_on_error(result);
#endif
}

void avs_config_sntp_ea_set(core_property_node_t *node, bool on)
{
    core_property_node_result_t result;

    log_assert(node);

#if AVS_VERSION <= 1306
    result = core_property_node_ext_u8_replace(node, "sntp/ea_on", on ? 1 : 0);
    core_property_node_fatal_on_error(result);
#else
    result = core_property_node_ext_bool_replace(node, "sntp/ea_on", on);
    core_property_node_fatal_on_error(result);
#endif
}

void avs_config_log_level_set(core_property_node_t *node, const char *level)
{
    core_property_node_result_t result;

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

    result = core_property_node_ext_u32_replace(node, "log/level", level_value);
    core_property_node_fatal_on_error(result);
#else
    result = core_property_node_ext_str_replace(node, "log/level", level);
    core_property_node_fatal_on_error(result);
#endif
}

void avs_config_log_name_set(core_property_node_t *node, const char *name)
{
    core_property_node_result_t result;

    log_assert(node);
    log_assert(name);

    result = core_property_node_ext_str_replace(node, "log/name", name);
    core_property_node_fatal_on_error(result);
}

void avs_config_log_file_set(core_property_node_t *node, const char *file)
{
    core_property_node_result_t result;

    log_assert(node);
    log_assert(file);

    result = core_property_node_ext_str_replace(node, "log/file", file);
    core_property_node_fatal_on_error(result);
}

void avs_config_log_buffer_size_set(core_property_node_t *node, uint32_t size)
{
    core_property_node_result_t result;

    log_assert(node);

    result = core_property_node_ext_u32_replace(node, "log/sz_buf", size);
    core_property_node_fatal_on_error(result);
}

void avs_config_log_output_delay_set(
    core_property_node_t *node, uint16_t delay_ms)
{
    core_property_node_result_t result;

    log_assert(node);

    result =
        core_property_node_ext_u16_replace(node, "log/output_delay", delay_ms);
    core_property_node_fatal_on_error(result);
}

void avs_config_log_enable_console_set(core_property_node_t *node, bool enable)
{
    core_property_node_result_t result;

    log_assert(node);

#if AVS_VERSION <= 1306
    result = core_property_node_ext_u8_replace(
        node, "log/enable_console", enable ? 1 : 0);
    core_property_node_fatal_on_error(result);
#else
    result =
        core_property_node_ext_bool_replace(node, "log/enable_console", enable);
    core_property_node_fatal_on_error(result);
#endif
}

void avs_config_log_enable_sci_set(core_property_node_t *node, bool enable)
{
    core_property_node_result_t result;

    log_assert(node);

#if AVS_VERSION <= 1306
    result = core_property_node_ext_u8_replace(
        node, "log/enable_netsci", enable ? 1 : 0);
    core_property_node_fatal_on_error(result);
#else
    result =
        core_property_node_ext_bool_replace(node, "log/enable_netsci", enable);
    core_property_node_fatal_on_error(result);
#endif
}

void avs_config_log_enable_net_set(core_property_node_t *node, bool enable)
{
    core_property_node_result_t result;

    log_assert(node);

#if AVS_VERSION <= 1306
    result = core_property_node_ext_u8_replace(
        node, "log/enable_netlog", enable ? 1 : 0);
    core_property_node_fatal_on_error(result);
#else
    result =
        core_property_node_ext_bool_replace(node, "log/enable_netlog", enable);
    core_property_node_fatal_on_error(result);
#endif
}

void avs_config_log_enable_file_set(core_property_node_t *node, bool enable)
{
    core_property_node_result_t result;

    log_assert(node);

#if AVS_VERSION <= 1306
    result = core_property_node_ext_u8_replace(
        node, "log/enable_file", enable ? 1 : 0);
    core_property_node_fatal_on_error(result);
#else
    result =
        core_property_node_ext_bool_replace(node, "log/enable_file", enable);
    core_property_node_fatal_on_error(result);
#endif
}

void avs_config_log_rotate_set(core_property_node_t *node, bool rotate)
{
    core_property_node_result_t result;

    log_assert(node);

#if AVS_VERSION <= 1306
    result =
        core_property_node_ext_u8_replace(node, "log/rotate", rotate ? 1 : 0);
    core_property_node_fatal_on_error(result);
#else
    result = core_property_node_ext_bool_replace(node, "log/rotate", rotate);
    core_property_node_fatal_on_error(result);
#endif
}

void avs_config_log_append_set(core_property_node_t *node, bool append)
{
    core_property_node_result_t result;

    log_assert(node);

#if AVS_VERSION <= 1306
    result =
        core_property_node_ext_u8_replace(node, "log/append", append ? 1 : 0);
    core_property_node_fatal_on_error(result);
#else
    result = core_property_node_ext_bool_replace(node, "log/append", append);
    core_property_node_fatal_on_error(result);
#endif
}

void avs_config_log_count_set(core_property_node_t *node, uint16_t count)
{
    core_property_node_result_t result;

    log_assert(node);

    result = core_property_node_ext_u16_replace(node, "log/gen", count);
    core_property_node_fatal_on_error(result);
}

void avs_config_set_log_level(
    core_property_node_t *node, enum core_log_bt_log_level loglevel)
{
    const char *str;

    log_assert(node);

    switch (loglevel) {
        case CORE_LOG_BT_LOG_LEVEL_OFF:
            str = "disable";
            break;

        case CORE_LOG_BT_LOG_LEVEL_FATAL:
            str = "fatal";
            break;

        case CORE_LOG_BT_LOG_LEVEL_WARNING:
            str = "warn";
            break;

        case CORE_LOG_BT_LOG_LEVEL_INFO:
            str = "info";
            break;

        case CORE_LOG_BT_LOG_LEVEL_MISC:
            str = "misc";
            break;

        default:
            log_fatal("Unsupported log level: %d", loglevel);
            break;
    }

    avs_config_log_level_set(node, str);
}

void avs_config_local_fs_path_dev_nvram_and_raw_set(
    core_property_node_t *node, const char *dev_nvram_raw_path)
{
    char path_dev_raw[MAX_PATH];
    char path_dev_nvram[MAX_PATH];

    core_property_node_t fs_node;
    core_property_node_t mounttable_node;
    core_property_node_t vfs_node;
    core_property_node_t tmp_node;
    core_property_node_result_t result;

    log_assert(node);
    log_assert(dev_nvram_raw_path);

    str_cpy(path_dev_raw, sizeof(path_dev_raw), dev_nvram_raw_path);
    str_cat(path_dev_raw, sizeof(path_dev_raw), "/dev/raw");

    str_cpy(path_dev_nvram, sizeof(path_dev_nvram), dev_nvram_raw_path);
    str_cat(path_dev_nvram, sizeof(path_dev_nvram), "/dev/nvram");

    result = core_property_node_search(node, "fs", &fs_node);

    if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        log_fatal("Cannot find 'fs' node in avs config");
    } else {
        core_property_node_fatal_on_error(result);
    }

    // Check if "new" mounttable config is used for dev/nvram and dev/raw or
    // legacy config
    result =
        core_property_node_search(&fs_node, "mounttable", &mounttable_node);

    if (result == CORE_PROPERTY_NODE_RESULT_SUCCESS) {
        result = core_property_node_remove(&mounttable_node);
        core_property_node_fatal_on_error(result);

        result = core_property_node_void_create(
            &fs_node, "mounttable", &mounttable_node);
        core_property_node_fatal_on_error(result);

        result =
            core_property_node_void_create(&mounttable_node, "vfs", &vfs_node);
        core_property_node_fatal_on_error(result);

        result = core_property_node_attr_create(
            &vfs_node, "name", "boot", &tmp_node);
        core_property_node_fatal_on_error(result);
        result = core_property_node_attr_create(
            &vfs_node, "fstype", "fs", &tmp_node);
        core_property_node_fatal_on_error(result);
        result = core_property_node_attr_create(
            &vfs_node, "src", path_dev_raw, &tmp_node);
        core_property_node_fatal_on_error(result);
        result = core_property_node_attr_create(
            &vfs_node, "dest", "/dev/raw", &tmp_node);
        core_property_node_fatal_on_error(result);
        result = core_property_node_attr_create(
            &vfs_node, "opt", "vf=1,posix=1", &tmp_node);
        core_property_node_fatal_on_error(result);

        result =
            core_property_node_void_create(&mounttable_node, "vfs", &vfs_node);
        core_property_node_fatal_on_error(result);

        result = core_property_node_attr_create(
            &vfs_node, "name", "boot", &tmp_node);
        core_property_node_fatal_on_error(result);
        result = core_property_node_attr_create(
            &vfs_node, "fstype", "fs", &tmp_node);
        core_property_node_fatal_on_error(result);
        result = core_property_node_attr_create(
            &vfs_node, "src", path_dev_nvram, &tmp_node);
        core_property_node_fatal_on_error(result);
        result = core_property_node_attr_create(
            &vfs_node, "dest", "/dev/nvram", &tmp_node);
        core_property_node_fatal_on_error(result);
        result = core_property_node_attr_create(
            &vfs_node, "opt", "vf=1,posix=1", &tmp_node);
        core_property_node_fatal_on_error(result);
    } else if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        result = core_property_node_ext_str_replace(
            &fs_node, "nvram/device", path_dev_nvram);
        core_property_node_fatal_on_error(result);
        result =
            core_property_node_ext_str_replace(&fs_node, "nvram/fstype", "fs");
        core_property_node_fatal_on_error(result);
        result = core_property_node_ext_str_replace(
            &fs_node, "nvram/option", "vf=1,posix=1");
        core_property_node_fatal_on_error(result);

        result = core_property_node_ext_str_replace(
            &fs_node, "raw/device", path_dev_raw);
        core_property_node_fatal_on_error(result);
        result =
            core_property_node_ext_str_replace(&fs_node, "raw/fstype", "fs");
        core_property_node_fatal_on_error(result);
        result = core_property_node_ext_str_replace(
            &fs_node, "raw/option", "vf=1,posix=1");
        core_property_node_fatal_on_error(result);
    } else {
        core_property_node_fatal_on_error(result);
    }
}

void avs_config_vfs_mounttable_get(
    const core_property_node_t *node,
    struct avs_config_vfs_mounttable *mounttable)
{
    core_property_node_t fs_node;
    core_property_node_t mounttable_node;
    core_property_node_t cur;
    core_property_node_t tmp;
    core_property_node_result_t result;

    char mounttable_selector[128];
    char name[128];
    uint8_t pos;

    log_assert(node);
    log_assert(mounttable);

    result = core_property_node_search(node, "fs", &fs_node);

    if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        log_fatal("Cannot find 'fs' node in avs config");
    } else {
        core_property_node_fatal_on_error(result);
    }

    // Check if new mounttable config is used for dev/nvram and dev/raw or
    // legacy config
    result =
        core_property_node_search(&fs_node, "mounttable", &mounttable_node);

    memset(mounttable, 0, sizeof(*mounttable));
    pos = 0;

    if (result == CORE_PROPERTY_NODE_RESULT_SUCCESS) {
        result =
            core_property_node_search(&fs_node, "mounttable_selector", &cur);

        if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
            log_fatal("Missing 'mounttable_selector' on mounttable");
        } else {
            core_property_node_fatal_on_error(result);
        }

        result = core_property_node_str_read(
            &cur, mounttable_selector, sizeof(mounttable_selector));

        if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
            log_fatal("Reading 'mounttable_selector' failed");
        }

        log_misc("Mounttable selector: %s", mounttable_selector);

        result = core_property_node_child_get(&mounttable_node, &cur);

        if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
            core_property_node_fatal_on_error(result);
        }

        while (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
            result = core_property_node_name_get(&cur, name, sizeof(name));
            core_property_node_fatal_on_error(result);

            if (str_eq(name, "vfs")) {
                if (pos >= AVS_CONFIG_MOUNTTABLE_MAX_ENTRIES) {
                    log_warning(
                        "Exceeding max number of supported mounttable entries "
                        "(%d), ignoring remaining",
                        pos);
                    break;
                }

                result = core_property_node_ext_attr_read(
                    &cur, "name@", name, sizeof(name));

                if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
                    log_fatal("Missing 'name' attribute on vfs node");
                } else {
                    core_property_node_fatal_on_error(result);
                }

                if (str_eq(name, mounttable_selector)) {
                    result = core_property_node_ext_attr_read(
                        &cur,
                        "fstype@",
                        mounttable->entry[pos].fstype,
                        sizeof(mounttable->entry[pos].fstype));

                    if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
                        // default
                        str_cpy(
                            mounttable->entry[pos].fstype,
                            sizeof(mounttable->entry[pos].fstype),
                            "fs");
                    } else {
                        core_property_node_fatal_on_error(result);
                    }

                    result = core_property_node_ext_attr_read(
                        &cur,
                        "src@",
                        mounttable->entry[pos].src,
                        sizeof(mounttable->entry[pos].src));

                    if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
                        log_fatal(
                            "Missing 'src' attribute on vfs node, name: %s",
                            name);
                    } else {
                        core_property_node_fatal_on_error(result);
                    }

                    result = core_property_node_ext_attr_read(
                        &cur,
                        "dst@",
                        mounttable->entry[pos].dst,
                        sizeof(mounttable->entry[pos].dst));

                    if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
                        log_fatal(
                            "Missing 'dst' attribute on vfs node, name: %s",
                            name);
                    } else {
                        core_property_node_fatal_on_error(result);
                    }

                    result = core_property_node_ext_attr_read(
                        &cur,
                        "opt@",
                        mounttable->entry[pos].opt,
                        sizeof(mounttable->entry[pos].opt));

                    if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
                        // optional
                    } else {
                        core_property_node_fatal_on_error(result);
                    }

                    pos++;
                }
            }

            result = core_property_node_next_sibling_get(&cur, &tmp);
            memcpy(&cur, &tmp, sizeof(core_property_node_t));

            if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
                core_property_node_fatal_on_error(result);
            }
        }
    } else if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        result = core_property_node_search(&fs_node, "nvram", &cur);

        if (result == CORE_PROPERTY_NODE_RESULT_SUCCESS) {
            result = core_property_node_ext_str_read(
                &cur,
                "fstype",
                mounttable->entry[pos].fstype,
                sizeof(mounttable->entry[pos].fstype));

            if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
                // default
                str_cpy(
                    mounttable->entry[pos].fstype,
                    sizeof(mounttable->entry[pos].fstype),
                    "fs");
            } else {
                core_property_node_fatal_on_error(result);
            }

            result = core_property_node_ext_str_read(
                &cur,
                "device",
                mounttable->entry[pos].src,
                sizeof(mounttable->entry[pos].src));

            if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
                log_fatal("Missing 'device' attribute on nvram node");
            } else {
                core_property_node_fatal_on_error(result);
            }

            str_cpy(
                mounttable->entry[pos].dst,
                sizeof(mounttable->entry[pos].dst),
                "/dev/nvram");

            result = core_property_node_ext_str_read(
                &cur,
                "opt",
                mounttable->entry[pos].opt,
                sizeof(mounttable->entry[pos].opt));

            if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
                // optional
            } else {
                core_property_node_fatal_on_error(result);
            }

            pos++;
        } else if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
            core_property_node_fatal_on_error(result);
        }

        result = core_property_node_search(&fs_node, "raw", &cur);

        if (result == CORE_PROPERTY_NODE_RESULT_SUCCESS) {
            result = core_property_node_ext_str_read(
                &cur,
                "fstype",
                mounttable->entry[pos].fstype,
                sizeof(mounttable->entry[pos].fstype));

            if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
                // default
                str_cpy(
                    mounttable->entry[pos].fstype,
                    sizeof(mounttable->entry[pos].fstype),
                    "fs");
            } else {
                core_property_node_fatal_on_error(result);
            }

            result = core_property_node_ext_str_read(
                &cur,
                "device",
                mounttable->entry[pos].src,
                sizeof(mounttable->entry[pos].src));

            if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
                log_fatal("Missing 'device' attribute on raw node");
            } else {
                core_property_node_fatal_on_error(result);
            }

            str_cpy(
                mounttable->entry[pos].dst,
                sizeof(mounttable->entry[pos].dst),
                "/dev/raw");

            result = core_property_node_ext_str_read(
                &cur,
                "opt",
                mounttable->entry[pos].opt,
                sizeof(mounttable->entry[pos].opt));

            if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
                // optional
            } else {
                core_property_node_fatal_on_error(result);
            }

            pos++;
        } else if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
            core_property_node_fatal_on_error(result);
        }
    } else {
        core_property_node_fatal_on_error(result);
    }

    mounttable->num_entries = pos;
}