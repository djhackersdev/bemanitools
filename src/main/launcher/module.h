#ifndef LAUNCHER_MODULE_H
#define LAUNCHER_MODULE_H

#include "imports/avs.h"

#include "launcher/ea3-ident-config.h"

struct module;

void module_load(const char *path, struct module *module);
void module_unresolved_load(
    const char *path,
    struct module *module);
HMODULE module_handle_get(const struct module *module);
void module_resolve(const struct module *module);
void module_init_invoke(
    struct ea3_ident_config *ea3_ident_config,
    struct property_node *app_params_node,
    const struct module *module);
bool module_main_invoke(const struct module *module);
void module_free(struct module *module);

#endif
