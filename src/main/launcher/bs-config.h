#ifndef LAUNCHER_BS_CONFIG_H
#define LAUNCHER_BS_CONFIG_H

#include <stdbool.h>
#include <windows.h>

#include "imports/avs.h"

struct bootstrap_startup_config {
    char avs_config_file[64];
    char launch_config_file[64];
    uint32_t avs_heap_size;
    uint32_t std_heap_size;
    char mount_table_selector[16];
    bool watcher_enable;
    bool timemachine_enable;

    char log_level[8];
    char log_name[64];
    char log_file[64];
    uint32_t log_bufsz;
    uint16_t log_output_delay_ms;
    bool log_enable_console;
    bool log_enable_sci;
    bool log_enable_net;
    bool log_enable_file;
    bool log_rotate;
    bool log_append;
    uint16_t log_count;

    uint8_t minidump_count;
    bool minidump_continue;
    bool minidump_log;
    uint8_t minidump_type;
    char minidump_path[64];
    uint32_t minidump_symbufsz;
    char minidump_search_path[64];

    char module_file[64];
    char module_load_type[64];

    char ntdll_digest[16];
    uint32_t ntdll_size;
    uint32_t ntdll_ift_table;
    uint32_t ntdll_ift_insert;
    uint32_t ntdll_ift_remove;

    bool shield_enable;
    bool shield_verbose;
    bool shield_use_loadlibrary;
    char shield_logger[64];
    uint32_t shield_sleep_min;
    uint32_t shield_sleep_blur;
    uint32_t shield_tick_sleep;
    uint32_t shield_tick_error;
    uint8_t shield_overwork_threshold;
    uint32_t shield_overwork_delay;
    uint32_t shield_pause_delay;
    char shield_whitelist_file[64];
    char shield_unlimited_key[10];
    uint16_t shield_killer_port;

    char dongle_license_cn[32];
    char dongle_account_cn[32];
    char dongle_driver_dll[16];
    bool dongle_disable_gc;

    char drm_dll[64];
    char drm_device[64];
    char drm_mount[64];
    char drm_fstype[64];
    char drm_options[64];

    bool lte_enable;
    char lte_config_file[64];
    char lte_unlimited_key[10];

    char ssl_options[64];

    bool esign_enable;

    bool eamuse_enable;
    bool eamuse_sync;
    bool eamuse_enable_model;
    char eamuse_config_file[64];
    bool eamuse_updatecert_enable;
    uint32_t eamuse_updatecert_interval;
};

struct bootstrap_config {
    char release_code[16];
    struct bootstrap_startup_config startup;
};

void bootstrap_config_init(struct bootstrap_config *config);
bool bootstrap_config_from_property(
    struct bootstrap_config *config,
    struct property *prop,
    const char *profile);

#endif /* LAUNCHER_BS_CONFIG_H */
