#include "cconfig/cconfig-util.h"
#include "cconfig/cmd.h"

#include "cconfig/cconfig-hook.h"
#include "cconfig/cconfig-main.h"

bool cconfig_hook_config_init(
    struct cconfig *config,
    const char *usage_header,
    enum cconfig_cmd_usage_out cmd_usage_out)
{
    return cconfig_main_config_init(
        config, "--config", NULL, "--help", "-h", usage_header, cmd_usage_out);
}
