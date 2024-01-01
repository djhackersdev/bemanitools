#ifndef LAUNCHER_AVS_CONFIG_H
#define LAUNCHER_AVS_CONFIG_H

#include "imports/avs.h"

#include "launcher/bootstrap-config.h"

#include "util/log.h"

struct property* avs_config_load_from_file_path(const char *filepath);
struct property_node* avs_config_resolve_root_node(struct property *property);
void avs_config_set_mode_product(struct property *property, bool enable);
void avs_config_set_net_raw(struct property *property, bool enable);
void avs_config_set_net_eaudp(struct property *property, bool enable);
void avs_config_set_sntp_ea(struct property *property, bool on);
void avs_config_set_fs_root_device(struct property *property, const char *path);
void avs_config_set_logging(
    struct property *property,
    const struct bootstrap_config *config);
void avs_config_set_log_level(
        struct property *property,
        enum log_level loglevel);
void avs_config_set_local_fs_path_dev_nvram_and_raw(
        struct property* property,
        const char* dev_nvram_raw_path);

#endif