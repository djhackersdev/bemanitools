#ifndef LAUNCHER_AVS_CONFIG_H
#define LAUNCHER_AVS_CONFIG_H

#include "core/log-bt.h"

#include "core/property-node.h"
#include "core/property.h"

#include "launcher/bootstrap-config.h"

#define AVS_CONFIG_MOUNTTABLE_MAX_ENTRIES 16

struct avs_config_vfs_mounttable {
    struct {
        char fstype[64];
        char src[512];
        char dst[512];
        char opt[256];
    } entry[AVS_CONFIG_MOUNTTABLE_MAX_ENTRIES];

    uint8_t num_entries;
};

core_property_t *avs_config_load(const char *filepath);
void avs_config_root_get(
    const core_property_t *property, core_property_node_t *node);
core_property_t *avs_config_property_merge(
    const core_property_t *parent, const core_property_t *source);

void avs_config_fs_root_device_get(
    const core_property_node_t *node, char *buffer, size_t size);

void avs_config_mode_product_set(core_property_node_t *node, bool enable);
void avs_config_net_raw_set(core_property_node_t *node, bool enable);
void avs_config_net_eaudp_set(core_property_node_t *node, bool enable);
void avs_config_sntp_ea_set(core_property_node_t *node, bool on);
void avs_config_log_level_set(core_property_node_t *node, const char *level);
void avs_config_log_name_set(core_property_node_t *node, const char *name);
void avs_config_log_file_set(core_property_node_t *node, const char *file);
void avs_config_log_buffer_size_set(core_property_node_t *node, uint32_t size);
void avs_config_log_output_delay_set(
    core_property_node_t *node, uint16_t delay_ms);
void avs_config_log_enable_console_set(core_property_node_t *node, bool enable);
void avs_config_log_enable_sci_set(core_property_node_t *node, bool enable);
void avs_config_log_enable_net_set(core_property_node_t *node, bool enable);
void avs_config_log_enable_file_set(core_property_node_t *node, bool enable);
void avs_config_log_rotate_set(core_property_node_t *node, bool rotate);
void avs_config_log_append_set(core_property_node_t *node, bool append);
void avs_config_log_count_set(core_property_node_t *node, uint16_t count);

void avs_config_set_log_level(
    core_property_node_t *node, enum core_log_bt_log_level loglevel);
void avs_config_local_fs_path_dev_nvram_and_raw_set(
    core_property_node_t *node, const char *dev_nvram_raw_path);

void avs_config_vfs_mounttable_get(
    const core_property_node_t *node,
    struct avs_config_vfs_mounttable *mounttable);

#endif