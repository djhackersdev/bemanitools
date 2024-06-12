#ifndef LAUNCHER_APP_H
#define LAUNCHER_APP_H

#include <windows.h>

#include "core/property-node.h"

#include "imports/eapki.h"

#include "launcher/ea3-ident-config.h"

typedef struct app {
    char path[MAX_PATH];
    HMODULE module;
    dll_entry_init_t init;
    dll_entry_main_t main;
} app_t;

void app_load(const char *path, app_t *app);
void app_unresolved_load(const char *path, app_t *app);
HMODULE app_module_handle_get(const app_t *app);
void app_resolve(const app_t *app);
void app_init_invoke(
    const app_t *app,
    struct ea3_ident_config *ea3_ident_config,
    const core_property_node_t *app_params_node);
bool app_main_invoke(const app_t *app);
void app_free(app_t *app);

#endif
