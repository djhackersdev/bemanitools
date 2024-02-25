#include <string.h>

#include "cconfig/cconfig-util.h"
#include "cconfig/cmd.h"
#include "cconfig/conf.h"

#include "cconfig/cconfig-main.h"

#include "core/log.h"

#include "util/cmdline.h"

bool cconfig_main_config_init(
    struct cconfig *config,
    const char *config_parameter_name,
    const char *default_config_path,
    const char *help_parameter_name,
    const char *help_parameter_short_name,
    const char *usage_header,
    enum cconfig_cmd_usage_out cmd_usage_out)
{
    bool success;
    int argc;
    char **argv;
    enum cconfig_conf_error conf_error;
    const char *config_path;

    success = true;

    args_recover(&argc, &argv);

    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], help_parameter_short_name) ||
            !strcmp(argv[i], help_parameter_name)) {
            goto failure_usage;
        }
    }

    config_path = NULL;

    if (config_parameter_name != NULL) {
        for (int i = 0; i < argc; i++) {
            if (!strcmp(argv[i], config_parameter_name)) {
                if (i + 1 >= argc) {
                    log_fatal(
                        "--config parameter not followed by a config file "
                        "path param");
                    goto failure;
                }

                config_path = argv[i + 1];

                break;
            }
        }
    }

    if (config_path == NULL && default_config_path != NULL) {
        log_misc("Using default config path: %s", default_config_path);
        config_path = default_config_path;
    }

    if (config_path) {
        log_misc("Loading config file: %s", config_path);
        conf_error = cconfig_conf_load_from_file(config, config_path, false);

        if (conf_error == CCONFIG_CONF_ERROR_NO_SUCH_FILE) {
            /* Create default config */
            if (cconfig_conf_save_to_file(config, config_path) !=
                CCONFIG_CONF_SUCCESS) {
                log_fatal(
                    "Creating default config file '%s' failed", config_path);
                goto failure;
            } else {
                log_info(
                    "Default configuration '%s' created. Restart "
                    "application",
                    config_path);
                goto failure;
            }
        } else if (conf_error != CCONFIG_CONF_SUCCESS) {
            log_fatal(
                "Error loading config file '%s': %d", config_path, conf_error);
            goto failure;
        }

        log_misc("Config state after file loading:");
        cconfig_util_log(config, core_log_misc_impl_get());
    }

    log_misc("Parsing override config parameters from cmd");

    /* Override defaults or values loaded from file with values from cmd */
    if (!cconfig_cmd_parse(config, "-p", argc, argv, false)) {
        log_fatal("Error parsing cmd args for config values");
        goto failure_usage;
    }

    log_misc("Config state after cmd parameter overrides:");
    cconfig_util_log(config, core_log_misc_impl_get());

    goto success;

failure_usage:
    cconfig_cmd_print_usage(config, usage_header, cmd_usage_out);

failure:
    success = false;

success:
    args_free(argc, argv);

    return success;
}
