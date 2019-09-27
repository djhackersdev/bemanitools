#ifndef LAUNCHER_MODULE_H
#define LAUNCHER_MODULE_H

#include <windows.h>

#include "imports/avs.h"

struct module_context {
    HMODULE dll;
    char *path;
};

void module_context_init(struct module_context *module, const char *path);
bool module_context_invoke_init(const struct module_context *module,
        char *sidcode, struct property_node *app_config);
bool module_context_invoke_main(const struct module_context *module);
void module_context_fini(struct module_context *module);

#endif
