#ifndef LAUNCHER_OPTIONS_H
#define LAUNCHER_OPTIONS_H

#include <stdbool.h>
#include <stddef.h>

#include "util/array.h"
#include "util/log.h"

#include "launcher/bootstrap-config.h"

// Launcher options (cmd params) are limited to:
// - Options to run a (vanilla) game without additional launcher features, e.g. hooking
// - Options that are handy to have for development/debugging purpose, e.g. quickly switching on/off
//   logging levels
//
// Everything else is driven by a composable configuration file (launcher.xml)
struct options {
    struct options_launcher {
        const char *config_path;
    } launcher;

    struct options_bootstrap {
        const char *config_path;
        const char *selector;
    } bootstrap;

    struct options_log {
        enum log_level *level;
        const char *file_path;
    } log;

    struct options_eamuse {
        const char *softid;
        const char *pcbid;
        const char *service_url;
        bool *urlslash;
    } eamuse;

    struct options_hook {
        struct array hook_dlls;
        struct array before_hook_dlls;
        struct array iat_hook_dlls;
    } hook;

    struct options_debug {
        bool remote_debugger;
        bool log_property_configs;
    } debug;
};

void options_init(struct options *options);
bool options_read_cmdline(struct options *options, int argc, const char **argv);
void options_print_usage(void);
void options_fini(struct options *options);

#endif
