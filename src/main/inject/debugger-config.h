#ifndef INJECT_DEBUGGER_CONFIG_H
#define INJECT_DEBUGGER_CONFIG_H

#include <windows.h>

#include <stdbool.h>

#include "core/property-node.h"

#include "inject/debugger.h"

// https://learn.microsoft.com/en-us/troubleshoot/windows-client/shell-experience/command-line-string-limitation
#define WINDOWS_CMD_LINE_ARGS_MAX_LEN 8192

typedef struct debugger_config {
    struct debugger_app_config {
        char path[MAX_PATH];
        char args[WINDOWS_CMD_LINE_ARGS_MAX_LEN];
    } app;

    debugger_attach_type_t attach_type;
} debugger_config_t;

void debugger_config_init(debugger_config_t *config);

void debugger_config_load(
    const core_property_node_t *node, debugger_config_t *config);

#endif