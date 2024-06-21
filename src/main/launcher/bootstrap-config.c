#define LOG_MODULE "bootstrap-config"

#include <string.h>

#include "core/property-node-ext.h"
#include "core/property-node.h"

#include "iface-core/log.h"

#include "imports/avs.h"

#include "launcher/avs-config.h"
#include "launcher/bootstrap-config.h"

#include "util/defs.h"
#include "util/hex.h"
#include "util/str.h"

#define ROOT_NODE "/config"
#define MODULE_PATH_PREFIX "modules/"

#define NODE_MISSING_FATAL(subnode) \
    log_fatal("%s/%s: Node missing", ROOT_NODE, subnode);
#define NODE_STARTUP_MISSING_FATAL(profile) \
    log_fatal("%s/startup/%s: Node missing", ROOT_NODE, profile);
#define NODE_PROFILE_MISSING_FATAL(profile, subnode) \
    log_fatal("%s/%s/%s: Node missing", ROOT_NODE, profile, subnode);
#define NODE_PROFILE_LOADING_FATAL(profile, subnode) \
    log_fatal("%s/startup/%s/%s: Node loading", ROOT_NODE, profile, subnode);

#define DEFAULT_HEAP_SIZE 16777216

const char *const inherited_nodes[] = {
    "develop",
    "default",
    "log",
    "minidump",
    "boot",
    "drm",
    "ssl",
    "eamuse",
    "shield",
    "esign",
    "dongle",
    "lte",
};

static void _bootstrap_config_profile_node_verify(
    const core_property_node_t *node, const char *profile)
{
    core_property_node_t profile_node;
    core_property_node_result_t result;

    log_assert(node);
    log_assert(profile);

    result = core_property_node_search(node, profile, &profile_node);

    if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        NODE_STARTUP_MISSING_FATAL(profile);
    } else {
        core_property_node_fatal_on_error(result);
    }
}

static void _bootstrap_config_root_node_get(
    const core_property_t *property, core_property_node_t *node)
{
    core_property_node_result_t result;
    char node_name[128];

    log_assert(property);
    log_assert(node);

    result = core_property_root_node_get(property, node);
    core_property_node_fatal_on_error(result);

    result = core_property_node_name_get(node, node_name, sizeof(node_name));
    core_property_node_fatal_on_error(result);

    if (!str_eq(node_name, "config")) {
        NODE_MISSING_FATAL("");
    } else {
        core_property_node_fatal_on_error(result);
    }
}

static void _bootstrap_config_startup_node_get(
    const core_property_node_t *node, core_property_node_t *node_out)
{
    core_property_node_result_t result;

    log_assert(node);
    log_assert(node_out);

    result = core_property_node_search(node, "startup", node_out);

    if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        NODE_MISSING_FATAL("startup");
    } else {
        core_property_node_fatal_on_error(result);
    }
}

static void _bootstrap_config_inheritance_resolve(
    const core_property_node_t *startup_node, const char *profile_name)
{
    core_property_node_t startup_parent_node;
    core_property_node_t startup_profile_node;
    core_property_node_t tmp_node;
    core_property_node_result_t result;

    char inherit_name[64];
    int i;

    result = core_property_node_search(
        startup_node, profile_name, &startup_profile_node);

    if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        log_fatal(ROOT_NODE "/startup/%s: missing", profile_name);
    } else {
        core_property_node_fatal_on_error(result);
    }

    memcpy(
        &startup_parent_node,
        &startup_profile_node,
        sizeof(core_property_node_t));

    while (true) {
        result = core_property_node_attr_read(
            &startup_parent_node,
            "inherit",
            inherit_name,
            sizeof(inherit_name));

        if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
            break;
        } else {
            core_property_node_fatal_on_error(result);
        }

        result = core_property_node_search(
            startup_node, inherit_name, &startup_parent_node);

        if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
            NODE_STARTUP_MISSING_FATAL(inherit_name);
        } else {
            core_property_node_fatal_on_error(result);
        }

        for (i = 0; i < _countof(inherited_nodes); i++) {
            result = core_property_node_search(
                startup_node, inherited_nodes[i], &tmp_node);

            // if found, then continue; if not found, skip this, any other
            // errors go fatal
            if (result == CORE_PROPERTY_NODE_RESULT_SUCCESS) {
                continue;
            } else if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
                core_property_node_fatal_on_error(result);
            }

            result = core_property_node_search(
                &startup_parent_node, inherited_nodes[i], &tmp_node);

            if (result == CORE_PROPERTY_NODE_RESULT_SUCCESS) {
                log_misc(
                    ROOT_NODE "/startup/%s: merging %s...",
                    inherit_name,
                    inherited_nodes[i]);

                result =
                    core_property_node_copy(&startup_profile_node, &tmp_node);

                if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
                    log_fatal(
                        "Merging '%s' into '%s' failed",
                        inherited_nodes[i],
                        inherit_name);
                }
            } else if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
                core_property_node_fatal_on_error(result);
            }
        }
    }
}

static void _bootstrap_config_load_bootstrap_module_app_config(
    const core_property_node_t *profile_node,
    struct bootstrap_module_config *config)
{
    core_property_node_t app_node;
    core_property_node_result_t result;

    log_assert(profile_node);
    log_assert(config);

    result =
        core_property_node_search(profile_node, "component/param", &app_node);
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_extract(&app_node, &config->app_config);
    core_property_node_fatal_on_error(result);
}

static void _bootstrap_config_load_bootstrap_default_files_config(
    const char *profile_name,
    const core_property_node_t *profile_node,
    struct bootstrap_default_file_config *config)
{
    int i;
    core_property_node_t tmp;
    core_property_node_t child;
    core_property_node_result_t result;

    log_assert(profile_node);
    log_assert(config);

    result = core_property_node_search(profile_node, "default/file", &child);
    i = 0;

    if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        core_property_node_fatal_on_error(result);
    }

    while (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        if (i >= DEFAULT_FILE_MAX) {
            log_warning(
                "Currently not supporting more than %d default files, skipping "
                "remaining",
                i);
            break;
        }

        result = core_property_node_attr_read(
            &child, "src", config->file[i].src, sizeof(config->file[i].src));

        if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
            log_fatal(
                "Missing src attribute on default file node of profile %s",
                profile_name);
        } else {
            core_property_node_fatal_on_error(result);
        }

        result = core_property_node_attr_read(
            &child, "dst", config->file[i].dst, sizeof(config->file[i].dst));

        if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
            log_fatal(
                "Missing dst attribute on default file node of profile %s",
                profile_name);
        } else {
            core_property_node_fatal_on_error(result);
        }

        result = core_property_node_next_result_search(&child, &tmp);
        memcpy(&child, &tmp, sizeof(core_property_node_t));
        i++;

        if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
            core_property_node_fatal_on_error(result);
        }
    }
}

static void _bootstrap_config_load_bootstrap(
    const core_property_node_t *startup_node,
    const char *profile,
    struct bootstrap_startup_config *config)
{
    core_property_node_t profile_node;
    core_property_node_result_t result;

    result = core_property_node_search(startup_node, profile, &profile_node);

    if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        NODE_PROFILE_LOADING_FATAL(profile, "");
    } else {
        core_property_node_fatal_on_error(result);
    }

    _bootstrap_config_load_bootstrap_default_files_config(
        profile, &profile_node, &config->default_file);

    result = core_property_node_ext_str_read(
        &profile_node,
        "boot/file",
        config->boot.config_file,
        sizeof(config->boot.config_file));
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_u32_read(
        &profile_node, "boot/heap_avs", &config->boot.avs_heap_size);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_u32_read_or_default(
        &profile_node, "boot/heap_std", &config->boot.std_heap_size, 0);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_str_read_or_default(
        &profile_node,
        "boot/mounttable_selector",
        config->boot.mount_table_selector,
        sizeof(config->boot.mount_table_selector),
        "boot");
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_bool_read_or_default(
        &profile_node, "boot/watcher", &config->boot.watcher_enable, true);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_bool_read_or_default(
        &profile_node,
        "boot/timemachine",
        &config->boot.timemachine_enable,
        false);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_str_read_or_default(
        &profile_node,
        "boot/launch_path",
        config->boot.launch_config_file,
        sizeof(config->boot.launch_config_file),
        "/dev/raw/launch.xml");
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_str_read_or_default(
        &profile_node,
        "log/level",
        config->log.level,
        sizeof(config->log.level),
        "all");
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_str_read_or_default(
        &profile_node,
        "log/name",
        config->log.name,
        sizeof(config->log.name),
        "");
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_str_read_or_default(
        &profile_node,
        "log/file",
        config->log.file,
        sizeof(config->log.file),
        "");
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_u32_read_or_default(
        &profile_node, "log/sz_buf", &config->log.bufsz, 4096);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_u16_read_or_default(
        &profile_node, "log/output_delay", &config->log.output_delay_ms, 10);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_bool_read_or_default(
        &profile_node, "log/enable_console", &config->log.enable_console, true);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_bool_read_or_default(
        &profile_node, "log/enable_netsci", &config->log.enable_sci, false);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_bool_read_or_default(
        &profile_node, "log/enable_netlog", &config->log.enable_net, true);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_bool_read_or_default(
        &profile_node, "log/enable_file", &config->log.enable_file, true);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_bool_read_or_default(
        &profile_node, "log/rotate", &config->log.rotate, true);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_bool_read_or_default(
        &profile_node, "log/append", &config->log.append, false);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_u16_read_or_default(
        &profile_node, "log/gen", &config->log.count, 10);
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_u8_read_or_default(
        &profile_node, "minidump/gen", &config->minidump.count, 10);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_bool_read_or_default(
        &profile_node,
        "minidump/cont_debug",
        &config->minidump.continue_,
        false);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_bool_read_or_default(
        &profile_node, "minidump/echo_log", &config->minidump.log, true);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_u8_read_or_default(
        &profile_node, "minidump/dump_type", &config->minidump.type, 2);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_str_read_or_default(
        &profile_node,
        "minidump/path",
        config->minidump.path,
        sizeof(config->minidump.path),
        "/dev/raw/minidump");
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_u32_read_or_default(
        &profile_node, "minidump/sz_symbuf", &config->minidump.symbufsz, 32768);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_str_read_or_default(
        &profile_node,
        "minidump/search",
        config->minidump.search_path,
        sizeof(config->minidump.search_path),
        ".");
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_str_read(
        &profile_node,
        "component/file",
        config->module.file,
        sizeof(config->module.file));
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_str_read_or_default(
        &profile_node,
        "component/load_type",
        config->module.load_type,
        sizeof(config->module.load_type),
        "MEMORY");
    core_property_node_fatal_on_error(result);

    _bootstrap_config_load_bootstrap_module_app_config(
        &profile_node, &config->module);

    /* disabled until we implement PSMAP_TYPE_BIN
    PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_startup_config,
    ntdll_digest, "dlml/ntdll/hash", "")
    */
    result = core_property_node_ext_u32_read_or_default(
        &profile_node, "dlml/ntdll/size", &config->dlm_ntdll.size, 0);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_u32_read_or_default(
        &profile_node, "dlml/ntdll/ift_table", &config->dlm_ntdll.ift_table, 0);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_u32_read_or_default(
        &profile_node,
        "dlml/ntdll/insert_ift",
        &config->dlm_ntdll.ift_insert,
        0);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_u32_read_or_default(
        &profile_node,
        "dlml/ntdll/remove_ift",
        &config->dlm_ntdll.ift_remove,
        0);
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_bool_read_or_default(
        &profile_node, "shield/enable", &config->shield.enable, true);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_bool_read_or_default(
        &profile_node, "shield/verbose", &config->shield.verbose, false);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_bool_read_or_default(
        &profile_node,
        "shield/use_loadlibrary",
        &config->shield.use_loadlibrary,
        false);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_str_read_or_default(
        &profile_node,
        "shield/use_loadlibrary",
        config->shield.logger,
        sizeof(config->shield.logger),
        "");
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_u32_read_or_default(
        &profile_node, "shield/sleepmin", &config->shield.sleep_min, 10);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_u32_read_or_default(
        &profile_node, "shield/sleepblur", &config->shield.sleep_blur, 90);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_str_read_or_default(
        &profile_node,
        "shield/whitelist",
        config->shield.whitelist_file,
        sizeof(config->shield.whitelist_file),
        "prop/whitelist.csv");
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_u32_read_or_default(
        &profile_node, "shield/ticksleep", &config->shield.tick_sleep, 100);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_u32_read_or_default(
        &profile_node, "shield/tickerror", &config->shield.tick_error, 1000);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_u8_read_or_default(
        &profile_node,
        "shield/overwork_threshold",
        &config->shield.overwork_threshold,
        50);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_u32_read_or_default(
        &profile_node,
        "shield/overwork_delay",
        &config->shield.overwork_delay,
        100);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_u32_read_or_default(
        &profile_node, "shield/pause_delay", &config->shield.pause_delay, 1000);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_str_read_or_default(
        &profile_node,
        "shield/unlimited_key",
        config->shield.unlimited_key,
        sizeof(config->shield.unlimited_key),
        "");
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_u16_read_or_default(
        &profile_node, "shield_killer/port", &config->shield.killer_port, 5001);
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_str_read_or_default(
        &profile_node,
        "dongle/license",
        config->dongle.license_cn,
        sizeof(config->dongle.license_cn),
        "");
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_str_read_or_default(
        &profile_node,
        "dongle/account",
        config->dongle.account_cn,
        sizeof(config->dongle.account_cn),
        "");
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_str_read_or_default(
        &profile_node,
        "dongle/pkcs11_driver",
        config->dongle.driver_dll,
        sizeof(config->dongle.driver_dll),
        "eTPKCS11.dll");
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_bool_read_or_default(
        &profile_node, "dongle/disable_gc", &config->dongle.disable_gc, false);
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_str_read_or_default(
        &profile_node, "drm/dll", config->drm.dll, sizeof(config->drm.dll), "");
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_str_read_or_default(
        &profile_node,
        "drm/fstype",
        config->drm.fstype,
        sizeof(config->drm.fstype),
        "");
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_str_read_or_default(
        &profile_node,
        "drm/device",
        config->drm.device,
        sizeof(config->drm.device),
        "");
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_str_read_or_default(
        &profile_node,
        "drm/dst",
        config->drm.mount,
        sizeof(config->drm.mount),
        "/");
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_str_read_or_default(
        &profile_node,
        "drm/option",
        config->drm.options,
        sizeof(config->drm.options),
        "");
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_bool_read_or_default(
        &profile_node, "lte/enable", &config->lte.enable, false);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_str_read_or_default(
        &profile_node,
        "lte/file",
        config->lte.config_file,
        sizeof(config->lte.config_file),
        "/dev/nvram/lte-config.xml");
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_str_read_or_default(
        &profile_node,
        "lte/unlimited_key",
        config->lte.unlimited_key,
        sizeof(config->lte.unlimited_key),
        "");
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_str_read_or_default(
        &profile_node,
        "ssl/option",
        config->ssl.options,
        sizeof(config->ssl.options),
        "");
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_bool_read_or_default(
        &profile_node, "esign/enable", &config->esign.enable, false);
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_bool_read_or_default(
        &profile_node, "eamuse/enable", &config->eamuse.enable, true);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_bool_read_or_default(
        &profile_node, "eamuse/sync", &config->eamuse.sync, true);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_bool_read_or_default(
        &profile_node,
        "eamuse/enable_model",
        &config->eamuse.enable_model,
        false);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_str_read_or_default(
        &profile_node,
        "eamuse/file",
        config->eamuse.config_file,
        sizeof(config->eamuse.config_file),
        "/dev/nvram/ea3-config.xml");
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_bool_read_or_default(
        &profile_node,
        "eamuse/updatecert_enable",
        &config->eamuse.updatecert_enable,
        true);
    core_property_node_fatal_on_error(result);
    result = core_property_node_ext_u32_read_or_default(
        &profile_node,
        "eamuse/updatecert_interval",
        &config->eamuse.updatecert_interval,
        0);
    core_property_node_fatal_on_error(result);
}

void bootstrap_config_init(struct bootstrap_config *config)
{
    log_assert(config);

    memset(config, 0, sizeof(*config));
}

void bootstrap_config_load(
    const core_property_t *property,
    const char *profile,
    struct bootstrap_config *config)
{
    core_property_node_t root_node;
    core_property_node_t startup_node;
    core_property_node_result_t result;

    log_assert(property);
    log_assert(profile);
    log_assert(config);

    log_info(ROOT_NODE ": loading...");

    _bootstrap_config_root_node_get(property, &root_node);

    result = core_property_node_ext_str_read_or_default(
        &root_node,
        "/release_code",
        config->release_code,
        sizeof(config->release_code),
        "");

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        log_fatal(ROOT_NODE ": loading failed");
    }

    _bootstrap_config_startup_node_get(&root_node, &startup_node);

    _bootstrap_config_profile_node_verify(&startup_node, profile);

    _bootstrap_config_inheritance_resolve(&startup_node, profile);

    log_misc(ROOT_NODE "/startup/%s: loading merged result...", profile);

    core_property_node_log(&startup_node, log_misc_func);

    _bootstrap_config_load_bootstrap(&startup_node, profile, &config->startup);

    log_misc("Loading finished");
}