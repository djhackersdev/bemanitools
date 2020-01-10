#ifndef CCONFIG_MAIN_H
#define CCONFIG_MAIN_H

#include "cconfig/cconfig.h"
#include "cconfig/cmd.h"

/**
 * Init function for cconfig after all cconfig_util_set's have been called
 *
 * will parse a config file first (if specified)
 * then uses override config parameters from cmd
 */
bool cconfig_main_config_init(
    struct cconfig *config,
    const char *config_cmd_param_name,
    const char *config_default_path,
    const char *help_cmd_param_name,
    const char *help_cmd_param_shortname,
    const char *usage_header,
    enum cconfig_cmd_usage_out cmd_usage_out);

#endif
