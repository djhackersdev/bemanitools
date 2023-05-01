#ifndef LAUNCHER_OPTIONS_H
#define LAUNCHER_OPTIONS_H

#include <stdbool.h>
#include <stddef.h>

#include "util/array.h"

struct options {
    size_t std_heap_size;
    size_t avs_heap_size;
    const char *bootstrap_config_path;
    const char *bootstrap_selector;
    const char *app_config_path;
    const char *avs_config_path;
    const char *ea3_config_path;
    const char *ea3_ident_path;
    const char *softid;
    const char *pcbid;
    const char *module;
    const char *logfile;
    struct array hook_dlls;
    struct array before_hook_dlls;
    struct array iat_hook_dlls;
    bool remote_debugger;
    const char *override_service;
    bool override_urlslash_enabled;
    bool override_urlslash_value;
};

void options_init(struct options *options);
void options_read_early_cmdline(
    struct options *options, int argc, const char **argv);
bool options_read_cmdline(struct options *options, int argc, const char **argv);
void options_print_usage(void);
void options_fini(struct options *options);

#endif
