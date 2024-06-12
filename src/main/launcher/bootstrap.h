#ifndef LAUNCHER_BOOTSTRAP_H
#define LAUNCHER_BOOTSTRAP_H

#include "core/property.h"

#include "launcher/bootstrap-config.h"
#include "launcher/ea3-ident-config.h"

#include "util/array.h"

void bootstrap_init(bool log_property_configs);
void bootstrap_log_init(const struct bootstrap_log_config *config);
void bootstrap_default_files_create(
    const struct bootstrap_default_file_config *config);
void bootstrap_avs_init(
    const struct bootstrap_boot_config *config,
    const struct bootstrap_log_config *log_config,
    const core_property_t *override_property);
void bootstrap_eamuse_init(
    const struct bootstrap_eamuse_config *config,
    const struct ea3_ident_config *ea3_ident_config,
    const core_property_t *override_property);
HMODULE bootstrap_app_unresolved_init(
    const struct bootstrap_module_config *module_config);
void bootstrap_app_resolve_init();
void bootstrap_app_init(
    const struct bootstrap_module_config *module_config,
    struct ea3_ident_config *ea3_ident_config);
void bootstrap_app_run();
void bootstrap_app_fini();
void bootstrap_avs_fini();
void bootstrap_eamuse_fini(const struct bootstrap_eamuse_config *config);

#endif