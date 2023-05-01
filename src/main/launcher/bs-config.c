#define LOG_MODULE "bootstrap"
#include <string.h>

#include "imports/avs.h"

#include "launcher/bs-config.h"

#include "util/defs.h"
#include "util/hex.h"
#include "util/log.h"
#include "util/str.h"

// clang-format off
PSMAP_BEGIN(bootstrap_startup_psmap)
PSMAP_REQUIRED(PSMAP_TYPE_STR,  struct bootstrap_startup_config, avs_config_file,
    "boot/file")
PSMAP_REQUIRED(PSMAP_TYPE_U32,  struct bootstrap_startup_config, avs_heap_size,
    "boot/heap_avs")
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_startup_config, std_heap_size,
    "boot/heap_std", 0)
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_startup_config, mount_table_selector,
    "boot/mounttable_selector", "boot")
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_startup_config, watcher_enable,
    "boot/watcher", 1)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_startup_config, timemachine_enable,
    "boot/timemachine", 0)
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_startup_config, launch_config_file,
    "boot/launch_path", "/dev/raw/launch.xml")

PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_startup_config, log_level,
    "log/level", "all")
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_startup_config, log_name,
    "log/name", "")
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_startup_config, log_file,
    "log/file", "")
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_startup_config, log_bufsz,
    "log/sz_buf", 4096)
PSMAP_OPTIONAL(PSMAP_TYPE_U16,  struct bootstrap_startup_config, log_output_delay_ms,
    "log/output_delay", 10)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_startup_config, log_enable_console,
    "log/enable_console", 1)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_startup_config, log_enable_sci,
    "log/enable_netsci", 0)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_startup_config, log_enable_net,
    "log/enable_netlog", 1)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_startup_config, log_enable_file,
    "log/enable_file", 1)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_startup_config, log_rotate,
    "log/rotate", 1)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_startup_config, log_append,
    "log/append", 0)
PSMAP_OPTIONAL(PSMAP_TYPE_U16,  struct bootstrap_startup_config, log_count,
    "log/gen", 10)

PSMAP_OPTIONAL(PSMAP_TYPE_U8,   struct bootstrap_startup_config, minidump_count,
    "minidump/gen", 10)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_startup_config, minidump_continue,
    "minidump/cont_debug", 0)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_startup_config, minidump_log,
    "minidump/echo_log", 1)
PSMAP_OPTIONAL(PSMAP_TYPE_U8,   struct bootstrap_startup_config, minidump_type,
    "minidump/dump_type", 2)
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_startup_config, minidump_path,
    "minidump/path", "/dev/raw/minidump")
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_startup_config, minidump_symbufsz,
    "minidump/sz_symbuf", 32768)
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_startup_config, minidump_search_path,
    "minidump/search", ".")

PSMAP_REQUIRED(PSMAP_TYPE_STR,  struct bootstrap_startup_config, module_file,
    "component/file")
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_startup_config, module_load_type,
    "component/load_type", "MEMORY")

/* disabled until we implement PSMAP_TYPE_BIN
   PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_startup_config, ntdll_digest,
       "dlml/ntdll/hash", "")
 */
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_startup_config, ntdll_size,
    "dlml/ntdll/size", 0)
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_startup_config, ntdll_ift_table,
    "dlml/ntdll/ift_table", 0)
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_startup_config, ntdll_ift_insert,
    "dlml/ntdll/insert_ift", 0)
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_startup_config, ntdll_ift_remove,
    "dlml/ntdll/remove_ift", 0)

PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_startup_config, shield_enable,
    "shield/enable", 1)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_startup_config, shield_verbose,
    "shield/verbose", 0)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_startup_config, shield_use_loadlibrary,
    "shield/use_loadlibrary", 0)
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_startup_config, shield_logger,
    "shield/logger", "")
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_startup_config, shield_sleep_min,
    "shield/sleepmin", 10)
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_startup_config, shield_sleep_blur,
    "shield/sleepblur", 90)
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_startup_config, shield_whitelist_file,
    "shield/whitelist", "prop/whitelist.csv")
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_startup_config, shield_tick_sleep,
    "shield/ticksleep", 100)
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_startup_config, shield_tick_error,
    "shield/tickerror", 1000)
PSMAP_OPTIONAL(PSMAP_TYPE_U8,   struct bootstrap_startup_config, shield_overwork_threshold,
    "shield/overwork_threshold", 50)
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_startup_config, shield_overwork_delay,
    "shield/overwork_delay", 100)
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_startup_config, shield_pause_delay,
    "shield/pause_delay", 1000)
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_startup_config, shield_unlimited_key,
    "shield/unlimited_key", "")
PSMAP_OPTIONAL(PSMAP_TYPE_U16,  struct bootstrap_startup_config, shield_killer_port,
    "shield_killer/port", 5001)

PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_startup_config, dongle_license_cn,
    "dongle/license", "")
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_startup_config, dongle_account_cn,
    "dongle/account", "")
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_startup_config, dongle_driver_dll,
    "dongle/pkcs11_driver", "eTPKCS11.dll")
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_startup_config, dongle_disable_gc,
    "dongle/disable_gc", 0)

PSMAP_REQUIRED(PSMAP_TYPE_STR,  struct bootstrap_startup_config, drm_dll,
    "drm/dll")
PSMAP_REQUIRED(PSMAP_TYPE_STR,  struct bootstrap_startup_config, drm_fstype,
    "drm/fstype")
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_startup_config, drm_device,
    "drm/device", "")
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_startup_config, drm_mount,
    "drm/dst", "/")
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_startup_config, drm_options,
    "drm/option", "")

PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_startup_config, lte_enable,
    "lte/enable", 0)
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_startup_config, lte_config_file,
    "lte/file", "/dev/nvram/lte-config.xml")
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_startup_config, lte_unlimited_key,
    "lte/unlimited_key", "")

PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_startup_config, ssl_options,
    "ssl/option", "")

PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_startup_config, esign_enable,
    "esign/enable", 0)

PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_startup_config, eamuse_enable,
    "eamuse/enable", 1)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_startup_config, eamuse_sync,
    "eamuse/sync", 1)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_startup_config, eamuse_enable_model,
    "eamuse/enable_model", 0)
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_startup_config, eamuse_config_file,
    "eamuse/file", "/dev/nvram/ea3-config.xml")
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_startup_config, eamuse_updatecert_enable,
    "eamuse/updatecert_enable", 1)
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_startup_config, eamuse_updatecert_interval,
    "eamuse/updatecert_interval", 0)
PSMAP_END

PSMAP_BEGIN(bootstrap_psmap)
PSMAP_REQUIRED(PSMAP_TYPE_STR, struct bootstrap_config, release_code, "/release_code")
PSMAP_END
// clang-format on

#define ROOT_NODE "/config"
#define MODULE_PATH_PREFIX "modules/"

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

void bootstrap_config_init(struct bootstrap_config *config)
{
    memset(config, 0, sizeof(*config));
}

bool bootstrap_config_from_property(
    struct bootstrap_config *config,
    struct property *prop,
    const char *selector)
{
    struct property_node *bootstrap_config =
        property_search(prop, NULL, ROOT_NODE);
    if (!bootstrap_config) {
        log_warning(ROOT_NODE ": missing");
        return false;
    }
    log_misc(ROOT_NODE ": loading...");
    if (!property_psmap_import(
            NULL, bootstrap_config, config, bootstrap_psmap)) {
        log_warning(ROOT_NODE ": load failed");
        return false;
    }

    /* Setup root startup node */
    struct property_node *startup_root =
        property_search(NULL, bootstrap_config, "startup");
    if (!startup_root) {
        log_warning(ROOT_NODE "/startup: missing");
        return false;
    }
    struct property_node *startup_config =
        property_search(NULL, startup_root, selector);
    if (!startup_config) {
        log_warning(ROOT_NODE "/startup/%s: missing", selector);
        return false;
    }

    /* Resolve inheritance */
    struct property_node *startup_parent = startup_config;
    for (;;) {
        char inherit_name[64];
        int r = property_node_refer(
            NULL,
            startup_parent,
            "inherit@",
            PROPERTY_TYPE_ATTR,
            inherit_name,
            sizeof(inherit_name));
        if (r < 0) {
            break;
        }

        startup_parent = property_search(NULL, startup_root, inherit_name);
        if (!startup_parent) {
            log_warning(ROOT_NODE "/startup/%s: missing", inherit_name);
            return false;
        }

        for (int i = 0; i < _countof(inherited_nodes); i++) {
            if (property_search(NULL, startup_config, inherited_nodes[i])) {
                continue;
            }

            struct property_node *node =
                property_search(NULL, startup_parent, inherited_nodes[i]);
            if (node) {
                log_misc(
                    ROOT_NODE "/startup/%s: merging %s...",
                    inherit_name,
                    inherited_nodes[i]);
                property_node_clone(NULL, startup_config, node, TRUE);
            }
        }
    }

    /* Now parse the startup node */
    log_misc(ROOT_NODE "/startup/%s: loading merge result...", selector);
    if (!property_psmap_import(
            NULL, startup_config, &config->startup, bootstrap_startup_psmap)) {
        log_warning(ROOT_NODE "/startup/%s: load failed", selector);
        return false;
    }

    config->module_params =
        property_search(NULL, startup_config, "component/param");
    config->log_node = property_search(NULL, startup_config, "log");
    config->default_node = property_search(NULL, startup_config, "default");
    return true;
}

void bootstrap_config_update_avs(
    const struct bootstrap_config *config, struct property_node *avs_root)
{
    if (config->module_params) {
        property_remove(NULL, avs_root, "mode/product");
        property_node_create(
            NULL, avs_root, PROPERTY_TYPE_BOOL, "mode/product", 1);
        property_remove(NULL, avs_root, "net/enable_raw");
        property_node_create(
            NULL, avs_root, PROPERTY_TYPE_BOOL, "net/enable_raw", 1);
        property_remove(NULL, avs_root, "net/eaudp/enable");
        property_node_create(
            NULL, avs_root, PROPERTY_TYPE_BOOL, "net/eaudp/enable", 1);
        property_remove(NULL, avs_root, "sntp/ea_on");
        property_node_create(
            NULL, avs_root, PROPERTY_TYPE_BOOL, "sntp/ea_on", 1);
    }
    if (config->startup.drm_device[0]) {
        property_remove(NULL, avs_root, "fs/root/device");
        property_node_create(
            NULL,
            avs_root,
            PROPERTY_TYPE_STR,
            "fs/root/device",
            config->startup.drm_device);
    }
    if (config->log_node) {
        property_remove(NULL, avs_root, "log");
        property_node_clone(NULL, avs_root, config->log_node, TRUE);
    }
}

bool bootstrap_config_iter_default_file(
    struct bootstrap_config *config,
    struct bootstrap_default_file *default_file)
{
    if (!config->default_file) {
        config->default_file =
            property_search(NULL, config->default_node, "file");
    } else {
        config->default_file = property_node_traversal(
            config->default_file, TRAVERSE_NEXT_SEARCH_RESULT);
    }
    if (!config->default_file) {
        return false;
    }

    int r;
    r = property_node_refer(
        NULL,
        config->default_file,
        "src@",
        PROPERTY_TYPE_ATTR,
        &default_file->src,
        sizeof(default_file->src));
    if (r < 0) {
        return false;
    }
    r = property_node_refer(
        NULL,
        config->default_file,
        "dst@",
        PROPERTY_TYPE_ATTR,
        &default_file->dest,
        sizeof(default_file->dest));
    if (r < 0) {
        return false;
    }
    return true;
}
