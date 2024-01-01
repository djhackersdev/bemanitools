#define LOG_MODULE "eamuse"

#include "imports/avs-ea3.h"

#include "launcher/ea3-ident.h"
#include "launcher/eamuse.h"
#include "launcher/eamuse-config.h"
#include "launcher/property-util.h"
#include "launcher/options.h"

#include "util/log.h"

static struct property *_eamuse_property;

void eamuse_init(
    const struct bootstrap_eamuse_config* config,
    const struct ea3_ident* ea3_ident,
    bool override_urlslash_enabled,
    bool override_urlslash_value,
    const char *override_service_url,
    bool log_property_config)
{
    struct property_node *node;

    log_assert(config);
    log_assert(ea3_ident);
    log_assert(override_service_url);

    if (config->enable) {
        _eamuse_property = eamuse_config_load_from_avs_path(config->config_file);

        eamuse_config_inject_ea3_ident(_eamuse_property, ea3_ident);
        eamuse_config_inject_parameters(
            _eamuse_property,
            override_urlslash_enabled,
            override_urlslash_value,
            override_service_url);

        if (log_property_config) {
            log_misc("Property ea3-config");
            property_util_log(_eamuse_property);
        }

        node = eamuse_config_resolve_root_node(_eamuse_property);

        log_info("Booting ea3...");

        ea3_boot(node);

        log_misc("Booting ea3 done");
    } else {
        _eamuse_property = NULL;
    }
}

void eamuse_fini()
{
    if (_eamuse_property) {
        ea3_shutdown();
        property_util_free(_eamuse_property);
        _eamuse_property = NULL;
    }
}