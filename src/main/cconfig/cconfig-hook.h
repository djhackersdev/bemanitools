#ifndef CCONFIG_HOOK_H
#define CCONFIG_HOOK_H

#include "cconfig/cconfig.h"
#include "cconfig/cmd.h"

bool cconfig_hook_config_init(
    struct cconfig *config,
    const char *usage_header,
    enum cconfig_cmd_usage_out cmd_usage_out);

#endif