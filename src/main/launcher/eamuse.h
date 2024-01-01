#ifndef LAUNCHER_EAMUSE_H
#define LAUNCHER_EAMUSE_H

#include "launcher/bootstrap-config.h"
#include "launcher/ea3-ident.h"

void eamuse_init(
    const struct bootstrap_eamuse_config* config,
    const struct ea3_ident* ea3_ident,
    bool override_urlslash_enabled,
    bool override_urlslash_value,
    const char *override_service_url,
    bool log_property_config);

void eamuse_fini();

#endif