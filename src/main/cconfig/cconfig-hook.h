#ifndef CCONFIG_HOOK_H
#define CCONFIG_HOOK_H

#include "cconfig/cconfig.h"
#include "cconfig/cmd.h"

/**
 * Init function for cconfig for hooks
 * 
 * calls cconfig_main_config_init with some defaults for hook dlls
 * see: cconfig_main_config_init for more details
 */
bool cconfig_hook_config_init(
    struct cconfig *config,
    const char *usage_header,
    enum cconfig_cmd_usage_out cmd_usage_out);

#endif
