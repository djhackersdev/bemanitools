#define LOG_MODULE "bootstrap"

#include "core/log-bt.h"
#include "core/log-sink-file.h"
#include "core/log-sink-list.h"
#include "core/log-sink-null.h"
#include "core/log-sink-std.h"
#include "core/log.h"

#include "launcher/avs-config.h"
#include "launcher/avs.h"
#include "launcher/bootstrap-config.h"
#include "launcher/ea3-ident-config.h"
#include "launcher/eamuse-config.h"
#include "launcher/eamuse.h"
#include "launcher/launcher-config.h"
#include "launcher/module.h"
#include "launcher/property-util.h"

#include "util/str.h"

static bool _bootstrap_log_property_configs;
static struct module_context _bootstrap_module_context;

static void _bootstrap_eamuse_ea3_ident_config_inject(
    struct property_node *node, const struct ea3_ident_config *ea3_ident_config)
{
    eamuse_config_id_softid_set(node, ea3_ident_config->softid);
    eamuse_config_id_hardid_set(node, ea3_ident_config->hardid);
    eamuse_config_id_pcbid_set(node, ea3_ident_config->pcbid);
    eamuse_config_soft_model_set(node, ea3_ident_config->model);
    eamuse_config_soft_dest_set(node, ea3_ident_config->dest);
    eamuse_config_soft_spec_set(node, ea3_ident_config->spec);
    eamuse_config_soft_rev_set(node, ea3_ident_config->rev);
    eamuse_config_soft_ext_set(node, ea3_ident_config->ext);
}

static void
_bootstrap_avs_config_force_overrides_apply(struct property_node *node)
{
    log_assert(node);

    avs_config_mode_product_set(node, true);
    avs_config_net_raw_set(node, true);
    avs_config_net_eaudp_set(node, true);
    avs_config_sntp_ea_set(node, true);
}

static void _bootstrap_avs_config_log_overrides_apply(
    struct property_node *node, const struct bootstrap_log_config *log_config)
{
    log_assert(node);
    log_assert(log_config);

    avs_config_log_level_set(node, log_config->level);
    avs_config_log_name_set(node, log_config->name);
    avs_config_log_file_set(node, log_config->file);
    avs_config_log_buffer_size_set(node, log_config->bufsz);
    avs_config_log_output_delay_set(node, log_config->output_delay_ms);
    avs_config_log_enable_console_set(node, log_config->enable_console);
    avs_config_log_enable_sci_set(node, log_config->enable_sci);
    avs_config_log_enable_net_set(node, log_config->enable_net);
    avs_config_log_enable_file_set(node, log_config->enable_file);
    avs_config_log_rotate_set(node, log_config->rotate);
    avs_config_log_append_set(node, log_config->append);
    avs_config_log_count_set(node, log_config->count);
}

static enum core_log_bt_log_level _bootstrap_log_map_level(const char *level)
{
    if (str_eq(level, "fatal")) {
        return CORE_LOG_BT_LOG_LEVEL_FATAL;
    } else if (str_eq(level, "warning")) {
        return CORE_LOG_BT_LOG_LEVEL_WARNING;
    } else if (str_eq(level, "info")) {
        return CORE_LOG_BT_LOG_LEVEL_INFO;
    } else if (str_eq(level, "misc")) {
        return CORE_LOG_BT_LOG_LEVEL_MISC;
    } else if (str_eq(level, "all")) {
        return CORE_LOG_BT_LOG_LEVEL_MISC;
    } else if (str_eq(level, "disable")) {
        return CORE_LOG_BT_LOG_LEVEL_OFF;
    } else if (str_eq(level, "default")) {
        return CORE_LOG_BT_LOG_LEVEL_WARNING;
    } else {
        log_fatal("Unknown log level string %s", level);
    }
}

void bootstrap_init(bool log_property_configs)
{
    log_info("init");

    _bootstrap_log_property_configs = log_property_configs;

    log_misc("init done");
}

void bootstrap_log_init(const struct bootstrap_log_config *config)
{
    struct core_log_sink sinks[2];
    struct core_log_sink sink_composed;
    enum core_log_bt_log_level level;

    log_assert(config);

    log_info("log init");

    // Shutdown old setup
    core_log_bt_fini();

    if (config->enable_file && strlen(config->file) > 0 &&
        config->enable_console) {
        core_log_sink_std_out_open(true, &sinks[0]);
        core_log_sink_file_open(
            config->file,
            config->append,
            config->rotate,
            config->count,
            &sinks[1]);
        core_log_sink_list_open(sinks, 2, &sink_composed);
    } else if (config->enable_file && strlen(config->file) > 0) {
        core_log_sink_file_open(
            config->file,
            config->append,
            config->rotate,
            config->count,
            &sink_composed);
    } else if (config->enable_console) {
        core_log_sink_std_out_open(true, &sink_composed);
    } else {
        core_log_sink_null_open(&sink_composed);
    }

    core_log_bt_init(&sink_composed);

    level = _bootstrap_log_map_level(config->level);
    core_log_bt_level_set(level);

    log_misc("log init done");
}

void bootstrap_default_files_create(
    const struct bootstrap_default_file_config *config)
{
    log_assert(config);

    log_info("default files create");

    for (int i = 0; i < DEFAULT_FILE_MAX; i++) {
        if (strlen(config->file[i].src) > 0 &&
            strlen(config->file[i].dst) > 0) {
            avs_fs_file_copy(config->file[i].src, config->file[i].dst);
        }
    }

    log_misc("default files create done");
}

void bootstrap_avs_init(
    const struct bootstrap_boot_config *config,
    const struct bootstrap_log_config *log_config,
    struct property *override_property)
{
    struct property *file_property;
    struct property *merged_property;
    struct property_node *root_node;

    log_assert(config);
    log_assert(log_config);
    log_assert(override_property);

    log_info("avs init");

    file_property = avs_config_load(config->config_file);

    if (_bootstrap_log_property_configs) {
        log_misc("avs-config from file: %s", config->config_file);
        property_util_log(file_property);
    }

    merged_property =
        avs_config_property_merge(file_property, override_property);

    property_util_free(file_property);

    if (_bootstrap_log_property_configs) {
        log_misc("avs-config merged with overrides");
        property_util_log(merged_property);
    }

    root_node = avs_config_root_get(merged_property);

    _bootstrap_avs_config_force_overrides_apply(root_node);
    _bootstrap_avs_config_log_overrides_apply(root_node, log_config);

    if (_bootstrap_log_property_configs) {
        log_misc("avs-config final");
        property_util_log(merged_property);
    }

    avs_fs_assert_root_device_exists(root_node);

    log_misc("Creating AVS file system directories...");

    avs_fs_mountpoints_fs_dirs_create(root_node);

    avs_init(root_node, config->avs_heap_size, config->std_heap_size);

    property_util_free(merged_property);

    log_misc("avs init done");
}

void bootstrap_eamuse_init(
    const struct bootstrap_eamuse_config *config,
    const struct ea3_ident_config *ea3_ident_config,
    struct property *override_property)
{
    struct property *file_property;
    struct property *merged_property;
    struct property_node *root_node;

    log_assert(config);
    log_assert(ea3_ident_config);
    log_assert(override_property);

    log_info("eamuse init");

    if (config->enable) {
        file_property = eamuse_config_avs_load(config->config_file);

        if (_bootstrap_log_property_configs) {
            log_misc("eamuse-config from file: %s", config->config_file);
            property_util_log(file_property);
        }

        merged_property = property_util_merge(file_property, override_property);

        property_util_free(file_property);

        if (_bootstrap_log_property_configs) {
            log_misc("eamuse-config merged with overrides");
            property_util_log(merged_property);
        }

        root_node = eamuse_config_root_get(merged_property);

        _bootstrap_eamuse_ea3_ident_config_inject(root_node, ea3_ident_config);

        if (_bootstrap_log_property_configs) {
            log_misc("eamuse-config final");
            property_util_log(merged_property);
        }

        eamuse_init(root_node);

        property_util_free(merged_property);
    } else {
        log_warning("Eamuse disabled");
    }

    log_misc("eamuse init done");
}

void bootstrap_module_init(
    const struct bootstrap_module_config *module_config,
    const struct array *iat_hook_dlls)
{
    log_assert(module_config);
    log_assert(iat_hook_dlls);

    log_info("module init");

    if (iat_hook_dlls->nitems > 0) {
        log_info(
            "Load game DLL with IAT hooks (%d): %s",
            (uint32_t) iat_hook_dlls->nitems,
            module_config->file);

        module_with_iat_hooks_init(
            &_bootstrap_module_context, module_config->file, iat_hook_dlls);
    } else {
        log_info("Load game DLL: %s", module_config->file);

        module_init(&_bootstrap_module_context, module_config->file);
    }

    log_misc("module init done");
}

void bootstrap_module_game_init(
    const struct bootstrap_module_config *module_config,
    struct ea3_ident_config *ea3_ident_config)
{
    struct property_node *node;

    log_assert(module_config);
    log_assert(ea3_ident_config);

    log_info("module game init");

    node = property_search(module_config->app_config, NULL, "/param");

    if (!node) {
        log_fatal("Missing param node on app-config");
    }

    if (_bootstrap_log_property_configs) {
        log_misc("app-config");
        property_util_node_log(node);
    }

    module_init_invoke(&_bootstrap_module_context, ea3_ident_config, node);

    log_misc("module game init done");
}

void bootstrap_module_game_run()
{
    log_info("module game run");

    module_main_invoke(&_bootstrap_module_context);
}

void bootstrap_module_game_fini()
{
    log_info("module game fini");

    module_fini(&_bootstrap_module_context);
}

void bootstrap_avs_fini()
{
    log_info("avs fini");

    avs_fini();
}

void bootstrap_eamuse_fini(const struct bootstrap_eamuse_config *config)
{
    log_info("eamuse fini");

    if (config->enable) {
        eamuse_fini();
    }
}