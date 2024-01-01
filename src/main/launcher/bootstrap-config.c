#define LOG_MODULE "bootstrap"
#include <string.h>

#include "imports/avs.h"

#include "launcher/avs-config.h"
#include "launcher/bootstrap-config.h"

#include "util/defs.h"
#include "util/hex.h"
#include "util/log.h"
#include "util/str.h"

// clang-format off
PSMAP_BEGIN(bootstrap_startup_avs_psmap)
PSMAP_REQUIRED(PSMAP_TYPE_STR,  struct bootstrap_avs_config, config_file,
    "boot/file")
PSMAP_REQUIRED(PSMAP_TYPE_U32,  struct bootstrap_avs_config, avs_heap_size,
    "boot/heap_avs")
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_avs_config, std_heap_size,
    "boot/heap_std", 0)
PSMAP_END

PSMAP_BEGIN(bootstrap_startup_boot_psmap)
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_boot_config, mount_table_selector,
    "boot/mounttable_selector", "boot")
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_boot_config, watcher_enable,
    "boot/watcher", 1)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_boot_config, timemachine_enable,
    "boot/timemachine", 0)
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_boot_config, launch_config_file,
    "boot/launch_path", "/dev/raw/launch.xml")
PSMAP_END

PSMAP_BEGIN(bootstrap_startup_log_psmap)
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_log_config, level,
    "log/level", "all")
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_log_config, name,
    "log/name", "")
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_log_config, file,
    "log/file", "")
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_log_config, bufsz,
    "log/sz_buf", 4096)
PSMAP_OPTIONAL(PSMAP_TYPE_U16,  struct bootstrap_log_config, output_delay_ms,
    "log/output_delay", 10)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_log_config, enable_console,
    "log/enable_console", 1)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_log_config, enable_sci,
    "log/enable_netsci", 0)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_log_config, enable_net,
    "log/enable_netlog", 1)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_log_config, enable_file,
    "log/enable_file", 1)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_log_config, rotate,
    "log/rotate", 1)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_log_config, append,
    "log/append", 0)
PSMAP_OPTIONAL(PSMAP_TYPE_U16,  struct bootstrap_log_config, count,
    "log/gen", 10)
PSMAP_END

PSMAP_BEGIN(bootstrap_startup_minidump_psmap)
PSMAP_OPTIONAL(PSMAP_TYPE_U8,   struct bootstrap_minidump_config, count,
    "minidump/gen", 10)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_minidump_config, continue_,
    "minidump/cont_debug", 0)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_minidump_config, log,
    "minidump/echo_log", 1)
PSMAP_OPTIONAL(PSMAP_TYPE_U8,   struct bootstrap_minidump_config, type,
    "minidump/dump_type", 2)
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_minidump_config, path,
    "minidump/path", "/dev/raw/minidump")
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_minidump_config, symbufsz,
    "minidump/sz_symbuf", 32768)
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_minidump_config, search_path,
    "minidump/search", ".")
PSMAP_END

PSMAP_BEGIN(bootstrap_startup_module_psmap)
PSMAP_REQUIRED(PSMAP_TYPE_STR,  struct bootstrap_module_config, file,
    "component/file")
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_module_config, load_type,
    "component/load_type", "MEMORY")
PSMAP_END

PSMAP_BEGIN(bootstrap_startup_dlm_psmap)
/* disabled until we implement PSMAP_TYPE_BIN
   PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_startup_config, ntdll_digest,
       "dlml/ntdll/hash", "")
 */
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_dlm_config, size,
    "dlml/ntdll/size", 0)
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_dlm_config, ift_table,
    "dlml/ntdll/ift_table", 0)
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_dlm_config, ift_insert,
    "dlml/ntdll/insert_ift", 0)
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_dlm_config, ift_remove,
    "dlml/ntdll/remove_ift", 0)
PSMAP_END

PSMAP_BEGIN(bootstrap_startup_shield_psmap)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_shield_config, enable,
    "shield/enable", 1)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_shield_config, verbose,
    "shield/verbose", 0)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_shield_config, use_loadlibrary,
    "shield/use_loadlibrary", 0)
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_shield_config, logger,
    "shield/logger", "")
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_shield_config, sleep_min,
    "shield/sleepmin", 10)
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_shield_config, sleep_blur,
    "shield/sleepblur", 90)
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_shield_config, whitelist_file,
    "shield/whitelist", "prop/whitelist.csv")
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_shield_config, tick_sleep,
    "shield/ticksleep", 100)
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_shield_config, tick_error,
    "shield/tickerror", 1000)
PSMAP_OPTIONAL(PSMAP_TYPE_U8,   struct bootstrap_shield_config, overwork_threshold,
    "shield/overwork_threshold", 50)
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_shield_config, overwork_delay,
    "shield/overwork_delay", 100)
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_shield_config, pause_delay,
    "shield/pause_delay", 1000)
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_shield_config, unlimited_key,
    "shield/unlimited_key", "")
PSMAP_OPTIONAL(PSMAP_TYPE_U16,  struct bootstrap_shield_config, killer_port,
    "shield_killer/port", 5001)
PSMAP_END

PSMAP_BEGIN(bootstrap_startup_dongle_psmap)
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_dongle_config, license_cn,
    "dongle/license", "")
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_dongle_config, account_cn,
    "dongle/account", "")
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_dongle_config, driver_dll,
    "dongle/pkcs11_driver", "eTPKCS11.dll")
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_dongle_config, disable_gc,
    "dongle/disable_gc", 0)
PSMAP_END

PSMAP_BEGIN(bootstrap_startup_drm_psmap)
PSMAP_REQUIRED(PSMAP_TYPE_STR,  struct bootstrap_drm_config, dll,
    "drm/dll")
PSMAP_REQUIRED(PSMAP_TYPE_STR,  struct bootstrap_drm_config, fstype,
    "drm/fstype")
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_drm_config, device,
    "drm/device", "")
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_drm_config, mount,
    "drm/dst", "/")
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_drm_config, options,
    "drm/option", "")
PSMAP_END

PSMAP_BEGIN(bootstrap_startup_lte_psmap)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_lte_config, enable,
    "lte/enable", 0)
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_lte_config, config_file,
    "lte/file", "/dev/nvram/lte-config.xml")
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_lte_config, unlimited_key,
    "lte/unlimited_key", "")
PSMAP_END

PSMAP_BEGIN(bootstrap_startup_ssl_psmap)
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_ssl_config, options,
    "ssl/option", "")
PSMAP_END

PSMAP_BEGIN(bootstrap_startup_esign_psmap)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_esign_config, enable,
    "esign/enable", 0)
PSMAP_END

PSMAP_BEGIN(bootstrap_startup_eamuse_psmap)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_eamuse_config, enable,
    "eamuse/enable", 1)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_eamuse_config, sync,
    "eamuse/sync", 1)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_eamuse_config, enable_model,
    "eamuse/enable_model", 0)
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_eamuse_config, config_file,
    "eamuse/file", "/dev/nvram/ea3-config.xml")
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct bootstrap_eamuse_config, updatecert_enable,
    "eamuse/updatecert_enable", 1)
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_eamuse_config, updatecert_interval,
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
            NULL, startup_config, &config->startup.avs, bootstrap_startup_avs_psmap)) {
        log_warning(ROOT_NODE "/startup/%s/boot (avs): load failed", selector);
        return false;
    }

    if (!property_psmap_import(
            NULL, startup_config, &config->startup.boot, bootstrap_startup_boot_psmap)) {
        log_warning(ROOT_NODE "/startup/%s/boot: load failed", selector);
        return false;
    }

    if (!property_psmap_import(
            NULL, startup_config, &config->startup.log, bootstrap_startup_log_psmap)) {
        log_warning(ROOT_NODE "/startup/%s/log: load failed", selector);
        return false;
    }

    if (!property_psmap_import(
            NULL, startup_config, &config->startup.minidump, bootstrap_startup_minidump_psmap)) {
        log_warning(ROOT_NODE "/startup/%s/minidump: load failed", selector);
        return false;
    }

    if (!property_psmap_import(
            NULL, startup_config, &config->startup.module, bootstrap_startup_module_psmap)) {
        log_warning(ROOT_NODE "/startup/%s/component: load failed", selector);
        return false;
    }

    if (!property_psmap_import(
            NULL, startup_config, &config->startup.dlm_ntdll, bootstrap_startup_dlm_psmap)) {
        log_warning(ROOT_NODE "/startup/%s/dlm/ntdll: load failed", selector);
        return false;
    }

    if (!property_psmap_import(
            NULL, startup_config, &config->startup.shield, bootstrap_startup_shield_psmap)) {
        log_warning(ROOT_NODE "/startup/%s/shield: load failed", selector);
        return false;
    }

    if (!property_psmap_import(
            NULL, startup_config, &config->startup.dongle, bootstrap_startup_dongle_psmap)) {
        log_warning(ROOT_NODE "/startup/%s/dongle: load failed", selector);
        return false;
    }

    if (!property_psmap_import(
            NULL, startup_config, &config->startup.drm, bootstrap_startup_drm_psmap)) {
        log_warning(ROOT_NODE "/startup/%s/drm: load failed", selector);
        return false;
    }

    if (!property_psmap_import(
            NULL, startup_config, &config->startup.lte, bootstrap_startup_lte_psmap)) {
        log_warning(ROOT_NODE "/startup/%s/lte: load failed", selector);
        return false;
    }

    if (!property_psmap_import(
            NULL, startup_config, &config->startup.ssl, bootstrap_startup_ssl_psmap)) {
        log_warning(ROOT_NODE "/startup/%s/ssl: load failed", selector);
        return false;
    }

    if (!property_psmap_import(
            NULL, startup_config, &config->startup.esign, bootstrap_startup_esign_psmap)) {
        log_warning(ROOT_NODE "/startup/%s/esign: load failed", selector);
        return false;
    }

    if (!property_psmap_import(
            NULL, startup_config, &config->startup.eamuse, bootstrap_startup_eamuse_psmap)) {
        log_warning(ROOT_NODE "/startup/%s/eamuse: load failed", selector);
        return false;
    }

    config->module_params =
        property_search(NULL, startup_config, "component/param");
    config->log_node = property_search(NULL, startup_config, "log");
    config->default_node = property_search(NULL, startup_config, "default");

    return true;
}

void bootstrap_config_update_avs(
    const struct bootstrap_config *config, struct property *avs_property)
{
    avs_config_set_mode_product(avs_property, true);
    avs_config_set_net_raw(avs_property, true);
    avs_config_set_net_eaudp(avs_property, true);
    avs_config_set_sntp_ea(avs_property, true);

    if (config->startup.drm.device[0]) {
        avs_config_set_fs_root_device(avs_property, config->startup.drm.device);
    }

    if (config->log_node) {
        avs_config_set_logging(avs_property, config);
    }
}

bool bootstrap_config_iter_default_file(
    struct bootstrap_config *config,
    struct bootstrap_default_file_config *default_file)
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
