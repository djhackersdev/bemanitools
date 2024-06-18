#define LOG_MODULE "bootstrap"

#include "core/log-bt.h"
#include "core/log-sink-async.h"
#include "core/log-sink-file.h"
#include "core/log-sink-list.h"
#include "core/log-sink-null.h"
#include "core/log-sink-std.h"

#include "core/property-ext.h"
#include "core/property-node.h"
#include "core/property.h"

#include "iface-core/log.h"

#include "launcher/app.h"
#include "launcher/avs-config.h"
#include "launcher/avs.h"
#include "launcher/bootstrap-config.h"
#include "launcher/ea3-ident-config.h"
#include "launcher/eamuse-config.h"
#include "launcher/eamuse.h"
#include "launcher/launcher-config.h"

#include "util/str.h"

static bool _bootstrap_log_property_configs;
static app_t _bootstrap_app;

static void _bootstrap_eamuse_ea3_ident_config_inject(
    core_property_node_t *node, const struct ea3_ident_config *ea3_ident_config)
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
_bootstrap_avs_config_force_overrides_apply(core_property_node_t *node)
{
    log_assert(node);

    avs_config_mode_product_set(node, true);
    avs_config_net_raw_set(node, true);
    avs_config_net_eaudp_set(node, true);
    avs_config_sntp_ea_set(node, true);
}

static void _bootstrap_avs_config_log_overrides_apply(
    core_property_node_t *node, const struct bootstrap_log_config *log_config)
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
    core_log_sink_t sinks[2];
    core_log_sink_t sink_composed;
    core_log_sink_t sink_async;
    enum core_log_bt_log_level level;

    log_assert(config);

    log_info("log init");

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

    // TODO have configurable parameters for these
    // TODO Allow for configuration of async logger on/off, default on
    core_log_sink_async_open(64 * 1024, 64, CORE_LOG_SINK_ASYNC_OVERFLOW_POLICY_DISCARD_NEW, &sink_composed, &sink_async);

    // TODO have configurable parameters for these
    core_log_bt_reinit(64 * 1024, &sink_async);

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
    const core_property_t *override_property)
{
    core_property_t *file_property;
    core_property_t *merged_property;
    core_property_node_t root_node;

    log_assert(config);
    log_assert(log_config);
    log_assert(override_property);

    log_info("avs init");

    file_property = avs_config_load(config->config_file);

    if (_bootstrap_log_property_configs) {
        log_misc("avs-config from file: %s", config->config_file);
        core_property_log(file_property, log_misc_func);
    }

    merged_property =
        avs_config_property_merge(file_property, override_property);

    core_property_free(&file_property);

    if (_bootstrap_log_property_configs) {
        log_misc("avs-config merged with overrides");
        core_property_log(merged_property, log_misc_func);
    }

    avs_config_root_get(merged_property, &root_node);

    _bootstrap_avs_config_force_overrides_apply(&root_node);
    _bootstrap_avs_config_log_overrides_apply(&root_node, log_config);

    if (_bootstrap_log_property_configs) {
        log_misc("avs-config final");
        core_property_log(merged_property, log_misc_func);
    }

    avs_fs_assert_root_device_exists(&root_node);

    log_misc("Creating AVS file system directories...");

    avs_fs_mountpoints_fs_dirs_create(&root_node);

    avs_init(&root_node, config->avs_heap_size, config->std_heap_size);

    core_property_free(&merged_property);

    log_misc("avs init done");
}

void bootstrap_eamuse_init(
    const struct bootstrap_eamuse_config *config,
    const struct ea3_ident_config *ea3_ident_config,
    const core_property_t *override_property)
{
    core_property_t *file_property;
    core_property_t *merged_property;
    core_property_node_t root_node;
    core_property_result_t prop_result;

    log_assert(config);
    log_assert(ea3_ident_config);
    log_assert(override_property);

    log_info("eamuse init");

    if (config->enable) {
        file_property = eamuse_config_avs_load(config->config_file);

        if (_bootstrap_log_property_configs) {
            log_misc("eamuse-config from file: %s", config->config_file);
            core_property_log(file_property, log_misc_func);
        }

        prop_result = core_property_ext_merge(
            file_property, override_property, &merged_property);
        core_property_fatal_on_error(prop_result);

        core_property_free(&file_property);

        if (_bootstrap_log_property_configs) {
            log_misc("eamuse-config merged with overrides");
            core_property_log(merged_property, log_misc_func);
        }

        eamuse_config_root_get(merged_property, &root_node);

        _bootstrap_eamuse_ea3_ident_config_inject(&root_node, ea3_ident_config);

        if (_bootstrap_log_property_configs) {
            log_misc("eamuse-config final");
            core_property_log(merged_property, log_misc_func);
        }

        eamuse_init(&root_node);

        core_property_free(&merged_property);
    } else {
        log_warning("Eamuse disabled");
    }

    log_misc("eamuse init done");
}

HMODULE bootstrap_app_unresolved_init(
    const struct bootstrap_module_config *module_config)
{
    log_assert(module_config);

    app_unresolved_load(module_config->file, &_bootstrap_app);

    return app_module_handle_get(&_bootstrap_app);
}

void bootstrap_app_resolve_init()
{
    app_resolve(&_bootstrap_app);
}

void bootstrap_app_init(
    const struct bootstrap_module_config *module_config,
    struct ea3_ident_config *ea3_ident_config)
{
    core_property_node_t node;
    core_property_result_t prop_result;
    core_property_node_result_t result;
    char node_name[128];

    log_assert(module_config);
    log_assert(ea3_ident_config);

    log_info("app init");

    prop_result = core_property_root_node_get(module_config->app_config, &node);
    core_property_fatal_on_error(prop_result);

    result = core_property_node_name_get(&node, node_name, sizeof(node_name));
    core_property_node_fatal_on_error(result);

    if (!str_eq(node_name, "param")) {
        log_fatal("Missing param node on app-config");
    } else {
        core_property_node_fatal_on_error(result);
    }

    if (_bootstrap_log_property_configs) {
        log_misc("app-config");
        core_property_node_log(&node, log_misc_func);
    }

    app_init_invoke(&_bootstrap_app, ea3_ident_config, &node);

    log_misc("app init done");
}

void bootstrap_app_run()
{
    app_main_invoke(&_bootstrap_app);
}

void bootstrap_app_fini()
{
    app_free(&_bootstrap_app);
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