#ifndef LAUNCHER_MODULE_H
#define LAUNCHER_MODULE_H

#include <windows.h>

#include "imports/avs.h"

#include "launcher/ea3-ident-config.h"

#include "util/array.h"

struct module_context {
    HMODULE dll;
    char *path;
};

void module_init(struct module_context *module, const char *path);
void module_with_iat_hooks_init(
    struct module_context *module,
    const char *path,
    const struct array *iat_hook_dlls);
void module_init_invoke(
    const struct module_context *module,
    struct ea3_ident_config *ea3_ident_config,
    struct property_node *app_params_node);
bool module_main_invoke(const struct module_context *module);
void module_fini(struct module_context *module);

#endif
