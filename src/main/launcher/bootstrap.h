#ifndef LAUNCHER_BOOTSTRAP_H
#define LAUNCHER_BOOTSTRAP_H

#include "launcher/bootstrap-config.h"
#include "launcher/ea3-ident-config.h"

#include "util/array.h"

void bootstrap_init(bool log_property_configs);
void bootstrap_log_init(const struct bootstrap_log_config *config);
void bootstrap_default_files_create(const struct bootstrap_default_file_config *config);
void bootstrap_avs_init(
    const struct bootstrap_boot_config *config,
    const struct bootstrap_log_config *log_config,
    struct property_node *override_node);
void bootstrap_eamuse_init(
    const struct bootstrap_eamuse_config *config,
    const struct ea3_ident_config *ea3_ident_config,
    struct property_node *override_node);
void bootstrap_module_init(
    const struct bootstrap_module_config *module_config,
    const struct array *iat_hook_dlls);
void bootstrap_module_game_init(
        const struct bootstrap_module_config *module_config,
        struct ea3_ident_config *ea3_ident_config);
void bootstrap_module_game_run();
void bootstrap_module_game_fini();
void bootstrap_avs_fini();
void bootstrap_eamuse_fini(const struct bootstrap_eamuse_config *config);

#endif