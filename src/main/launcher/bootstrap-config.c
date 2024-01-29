#define LOG_MODULE "bootstrap-config"

#include <string.h>

#include "imports/avs.h"

#include "launcher/avs-config.h"
#include "launcher/bootstrap-config.h"
#include "launcher/property-util.h"

#include "util/defs.h"
#include "util/hex.h"
#include "util/log.h"
#include "util/str.h"

// clang-format off
PSMAP_BEGIN(bootstrap_startup_boot_psmap)
PSMAP_REQUIRED(PSMAP_TYPE_STR,  struct bootstrap_boot_config, config_file,
    "boot/file")
PSMAP_REQUIRED(PSMAP_TYPE_U32,  struct bootstrap_boot_config, avs_heap_size,
    "boot/heap_avs")
PSMAP_OPTIONAL(PSMAP_TYPE_U32,  struct bootstrap_boot_config, std_heap_size,
    "boot/heap_std", 0)
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
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_drm_config, dll,
    "drm/dll", "")
PSMAP_OPTIONAL(PSMAP_TYPE_STR,  struct bootstrap_drm_config, fstype,
    "drm/fstype", "")
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
PSMAP_OPTIONAL(PSMAP_TYPE_STR, struct bootstrap_config, release_code, "/release_code", "")
PSMAP_END
// clang-format on

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
    struct property_node *node, const char *profile)
{
    struct property_node *profile_node;

    log_assert(node);
    log_assert(profile);

    profile_node = property_search(NULL, node, profile);

    if (!profile_node) {
        NODE_STARTUP_MISSING_FATAL(profile);
    }
}

static struct property_node *
_bootstrap_config_root_node_get(struct property *property)
{
    struct property_node *root_node;

    log_assert(property);

    root_node = property_search(property, NULL, ROOT_NODE);

    if (!root_node) {
        NODE_MISSING_FATAL("");
    }

    return root_node;
}

static struct property_node *
_bootstrap_config_startup_node_get(struct property_node *node)
{
    struct property_node *startup_node;

    log_assert(node);

    startup_node = property_search(NULL, node, "startup");

    if (!startup_node) {
        NODE_MISSING_FATAL("startup");
    }

    return startup_node;
}

static void _bootstrap_config_inheritance_resolve(
    struct property_node *startup_node, const char *profile_name)
{
    struct property_node *startup_parent_node;
    struct property_node *startup_profile_node;
    struct property_node *tmp_node;

    char inherit_name[64];
    avs_error error;

    startup_profile_node = property_search(NULL, startup_node, profile_name);

    if (!startup_profile_node) {
        log_fatal(ROOT_NODE "/startup/%s: missing", profile_name);
    }

    startup_parent_node = startup_profile_node;

    for (;;) {
        error = property_node_refer(
            NULL,
            startup_parent_node,
            "inherit@",
            PROPERTY_TYPE_ATTR,
            inherit_name,
            sizeof(inherit_name));

        if (AVS_IS_ERROR(error)) {
            break;
        }

        startup_parent_node = property_search(NULL, startup_node, inherit_name);

        if (!startup_parent_node) {
            NODE_STARTUP_MISSING_FATAL(inherit_name);
        }

        for (int i = 0; i < _countof(inherited_nodes); i++) {
            if (property_search(NULL, startup_node, inherited_nodes[i])) {
                continue;
            }

            tmp_node =
                property_search(NULL, startup_parent_node, inherited_nodes[i]);

            if (tmp_node) {
                log_misc(
                    ROOT_NODE "/startup/%s: merging %s...",
                    inherit_name,
                    inherited_nodes[i]);

                property_node_clone(NULL, startup_profile_node, tmp_node, TRUE);
            }
        }
    }
}

static void _bootstrap_config_load_bootstrap_module_app_config(
    struct property_node *profile_node, struct bootstrap_module_config *config)
{
    struct property_node *app_node;

    log_assert(profile_node);
    log_assert(config);

    app_node = property_search(NULL, profile_node, "component/param");

    config->app_config = property_util_clone(app_node);
}

static void _bootstrap_config_load_bootstrap_default_files_config(
    const char *profile_name,
    struct property_node *profile_node,
    struct bootstrap_default_file_config *config)
{
    int i;
    int result;
    struct property_node *child;

    log_assert(profile_node);
    log_assert(config);

    child = property_search(NULL, profile_node, "default/file");
    i = 0;

    while (child) {
        if (i >= DEFAULT_FILE_MAX) {
            log_warning(
                "Currently not supporting more than %d default files, skipping "
                "remaining",
                i);
            break;
        }

        result = property_node_refer(
            NULL,
            child,
            "src@",
            PROPERTY_TYPE_ATTR,
            &config->file[i].src,
            sizeof(config->file[i].src));

        if (result < 0) {
            log_fatal(
                "Missing src attribute on default file node of profile %s",
                profile_name);
        }

        result = property_node_refer(
            NULL,
            child,
            "dst@",
            PROPERTY_TYPE_ATTR,
            &config->file[i].dst,
            sizeof(config->file[i].dst));

        if (result < 0) {
            log_fatal(
                "Missing dst attribute on default file node of profile %s",
                profile_name);
        }

        child = property_node_traversal(child, TRAVERSE_NEXT_SEARCH_RESULT);
        i++;
    }
}

static void _bootstrap_config_load_bootstrap(
    struct property_node *startup_node,
    const char *profile,
    struct bootstrap_startup_config *config)
{
    struct property_node *profile_node;

    profile_node = property_search(NULL, startup_node, profile);

    if (!profile_node) {
        NODE_PROFILE_LOADING_FATAL(profile, "");
    }

    _bootstrap_config_load_bootstrap_default_files_config(
        profile, profile_node, &config->default_file);

    if (!property_psmap_import(
            NULL, profile_node, &config->boot, bootstrap_startup_boot_psmap)) {
        NODE_PROFILE_LOADING_FATAL(profile, "boot");
    }

    if (!property_psmap_import(
            NULL, profile_node, &config->log, bootstrap_startup_log_psmap)) {
        NODE_PROFILE_LOADING_FATAL(profile, "log");
    }

    if (!property_psmap_import(
            NULL,
            profile_node,
            &config->minidump,
            bootstrap_startup_minidump_psmap)) {
        NODE_PROFILE_LOADING_FATAL(profile, "minidump");
    }

    if (!property_psmap_import(
            NULL,
            profile_node,
            &config->module,
            bootstrap_startup_module_psmap)) {
        NODE_PROFILE_LOADING_FATAL(profile, "component");
    }

    _bootstrap_config_load_bootstrap_module_app_config(
        profile_node, &config->module);

    if (!property_psmap_import(
            NULL,
            profile_node,
            &config->dlm_ntdll,
            bootstrap_startup_dlm_psmap)) {
        NODE_PROFILE_LOADING_FATAL(profile, "dlm/ntdll");
    }

    if (!property_psmap_import(
            NULL,
            profile_node,
            &config->shield,
            bootstrap_startup_shield_psmap)) {
        NODE_PROFILE_LOADING_FATAL(profile, "shield");
    }

    if (!property_psmap_import(
            NULL,
            profile_node,
            &config->dongle,
            bootstrap_startup_dongle_psmap)) {
        NODE_PROFILE_LOADING_FATAL(profile, "dongle");
    }

    if (!property_psmap_import(
            NULL, profile_node, &config->drm, bootstrap_startup_drm_psmap)) {
        NODE_PROFILE_LOADING_FATAL(profile, "drm");
    }

    if (!property_psmap_import(
            NULL, profile_node, &config->lte, bootstrap_startup_lte_psmap)) {
        NODE_PROFILE_LOADING_FATAL(profile, "lte");
    }

    if (!property_psmap_import(
            NULL, profile_node, &config->ssl, bootstrap_startup_ssl_psmap)) {
        NODE_PROFILE_LOADING_FATAL(profile, "ssl");
    }

    if (!property_psmap_import(
            NULL,
            profile_node,
            &config->esign,
            bootstrap_startup_esign_psmap)) {
        NODE_PROFILE_LOADING_FATAL(profile, "esign");
    }

    if (!property_psmap_import(
            NULL,
            profile_node,
            &config->eamuse,
            bootstrap_startup_eamuse_psmap)) {
        NODE_PROFILE_LOADING_FATAL(profile, "eamuse");
    }
}

void bootstrap_config_init(struct bootstrap_config *config)
{
    log_assert(config);

    memset(config, 0, sizeof(*config));
}

void bootstrap_config_load(
    struct property *property,
    const char *profile,
    struct bootstrap_config *config)
{
    struct property_node *root_node;
    struct property_node *startup_node;

    log_assert(property);
    log_assert(profile);
    log_assert(config);

    log_info(ROOT_NODE ": loading...");

    root_node = _bootstrap_config_root_node_get(property);

    if (!property_psmap_import(NULL, root_node, config, bootstrap_psmap)) {
        log_fatal(ROOT_NODE ": loading failed");
    }

    startup_node = _bootstrap_config_startup_node_get(root_node);

    _bootstrap_config_profile_node_verify(startup_node, profile);

    _bootstrap_config_inheritance_resolve(startup_node, profile);

    log_misc(ROOT_NODE "/startup/%s: loading merged result...", profile);

    property_util_node_log(startup_node);

    _bootstrap_config_load_bootstrap(startup_node, profile, &config->startup);

    log_misc("Loading finished");
}