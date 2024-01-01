#include "eamuse.h"

#include "ea3-ident.h"
#include "property.h"
#include "options.h"

#include "imports/avs-ea3.h"
#include "util/log.h"

static const struct bootstrap_eamuse_config* eamuse_config;
static struct property_node *ea3_config_node;
static struct property *ea3_config_property;

static void ea3_config_setup(
    const struct ea3_ident *ea3_ident,
    const char *eamuse_config_file,
    bool override_urlslash_enabled,
    bool override_urlslash_value,
    const char *service_url,
    struct property **ea3_config_property)
{
    struct property_node *ea3_config_node;

    log_assert(ea3_ident);
    log_assert(eamuse_config_file);
    log_assert(ea3_config_property);

    log_misc("Preparing ea3 configuration...");

    log_misc("Loading ea3-config from file: %s", eamuse_config_file);

    *ea3_config_property = boot_property_load_avs(eamuse_config_file);
    ea3_config_node = property_search(*ea3_config_property, 0, "/ea3");

    if (ea3_config_node == NULL) {
        log_fatal("%s: /ea3 missing", eamuse_config_file);
    }

    ea3_ident_to_property(ea3_ident, *ea3_config_property);

    if (override_urlslash_enabled) {
        log_misc(
            "Overriding url_slash to: %d", override_urlslash_value);

        boot_property_node_replace_bool(
            *ea3_config_property,
            ea3_config_node,
            "network/url_slash",
            override_urlslash_value);
    }

    if (service_url) {
        log_misc("Overriding service url to: %s", service_url);

        boot_property_node_replace_str(
            *ea3_config_property,
            ea3_config_node,
            "network/services",
            service_url);
    }
}

void eamuse_init(
    const struct bootstrap_eamuse_config* config,
    const struct ea3_ident* ea3_ident,
    const struct options* options)
{
    log_assert(config);
    log_assert(ea3_ident);
    log_assert(options);

    eamuse_config = config;

    if (eamuse_config->enable) {
        ea3_config_setup(
            ea3_ident,
            eamuse_config->config_file,
            options->override_urlslash_enabled,
            options->override_urlslash_value,
            options->override_service,
            &ea3_config_property);

        if (options->log_property_configs) {
            log_misc("Property ea3-config");
            boot_property_log(ea3_config_property);
        }

        log_info("Booting ea3...");

        ea3_config_node = property_search(ea3_config_property, 0, "/ea3");

        log_assert(ea3_config_node);

        ea3_boot(ea3_config_node);
    } else {
        ea3_config_property = NULL;
        ea3_config_node = NULL;
    }
}

void eamuse_fini()
{
    if (eamuse_config->enable) {
        ea3_shutdown();
        ea3_config_node = NULL;
        boot_property_free(ea3_config_property);
    }
}