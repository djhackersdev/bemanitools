#ifndef INJECT_CONFIG_H
#define INJECT_CONFIG_H

#include <windows.h>

#include "inject/debug-config.h"
#include "inject/debugger-config.h"
#include "inject/hooks-config.h"
#include "inject/logger-config.h"

typedef struct inject_config {
    uint32_t version;

    hooks_config_t hooks;
    logger_config_t logger;
    debugger_config_t debugger;
    debug_config_t debug;
} inject_config_t;

void inject_config_init(struct inject_config *config);

void inject_config_file_load(const char *path, inject_config_t *config);

void inject_config_fini(inject_config_t *config);

#endif