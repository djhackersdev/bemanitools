#define LOG_MODULE "bootstrap-context"

#include "launcher/bs-config.h"
#include "launcher/property.h"

#include "util/log.h"
#include "util/str.h"

void bootstrap_context_init(
    const char *avs_config_path,
    const char *ea3_config_path,
    size_t std_heap_size,
    size_t avs_heap_size,
    const char *logfile,
    const char *module,
    struct bootstrap_config *config)
{
    log_assert(avs_config_path);
    log_assert(ea3_config_path);
    log_assert(module);

    log_info("Bootstrap from options");

    bootstrap_config_init(config);

    str_cpy(config->startup.avs_config_file, sizeof(config->startup.avs_config_file), avs_config_path);
    str_cpy(config->startup.eamuse_config_file, sizeof(config->startup.eamuse_config_file), ea3_config_path);

    config->startup.avs_heap_size = avs_heap_size;
    config->startup.std_heap_size = std_heap_size;

    if (logfile) {
        config->startup.log_enable_file = true;

        str_cpy(config->startup.log_file, sizeof(config->startup.log_file), logfile);
        str_cpy(config->startup.log_name, sizeof(config->startup.log_name), logfile);
    }

    str_cpy(config->startup.module_file, sizeof(config->startup.module_file), module);
}

void bootstrap_context_init_from_file(
    const char *config_path,
    const char *selector,
    struct bootstrap_config *config)
{
    struct property *property;

    log_assert(config_path);
    log_assert(selector);
    log_assert(config);

    log_info("Bootstrap from configuration %s with selector %s", config_path, selector);

    bootstrap_config_init(config);
    property = boot_property_load(config_path);

    log_info(
        "Loading bootstrap selector '%s'...", selector);
    
    if (!bootstrap_config_from_property(config, property, selector)) {
        log_fatal(
            "%s: could not load configuration for '%s'",
            config_path,
            selector);
    }

    boot_property_free(property);
}

void bootstrap_context_post_avs_setup(struct bootstrap_config *config)
{
    struct bootstrap_default_file default_file;
    struct avs_stat st;

    log_assert(config);

    log_misc("Bootstrap post AVS setup");

    while (bootstrap_config_iter_default_file(config, &default_file)) {
        log_misc("%s: copying from %s...", default_file.dest, default_file.src);

        if (!avs_fs_lstat(default_file.src, &st)) {
            log_fatal("Default file source %s does not exist or is not accessible", default_file.src);
        }

        if (avs_fs_copy(default_file.src, default_file.dest) < 0) {
            log_fatal(
                "%s: could not copy from %s",
                default_file.dest,
                default_file.src);
        }
    }
}