#ifndef LAUNCHER_MODULE_H
#define LAUNCHER_MODULE_H

#include "imports/avs.h"
#include "imports/eapki.h"

#include "launcher/ea3-ident-config.h"

struct module {
    char path[MAX_PATH];
    HMODULE module;
    dll_entry_init_t init;
    dll_entry_main_t main;
};

void module_load(const char *path, struct module *module);
void module_unresolved_load(const char *path, struct module *module);
HMODULE module_handle_get(const struct module *module);
void module_resolve(const struct module *module);
void module_init_invoke(
    const struct module *module,
    struct ea3_ident_config *ea3_ident_config,
    struct property_node *app_params_node);
bool module_main_invoke(const struct module *module);
void module_free(struct module *module);

#endif
