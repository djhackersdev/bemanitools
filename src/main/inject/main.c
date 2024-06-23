#define LOG_MODULE "main"

#include <stdio.h>

#include "core/boot.h"

#include "inject/inject-config.h"
#include "inject/inject.h"

static void _bootstrap_options(int argc, char **argv)
{
    // TODO use options here
    if (argc < 2) {
        printf("Not enough args\n");
        exit(1);
    }
}

static void _bootstrap_config(const char *path, inject_config_t *config)
{
    inject_config_init(config);
    inject_config_file_load(path, config);
}

int main(int argc, char **argv)
{
    const char *config_path;
    inject_config_t config;

    core_boot("inject");

    config_path = argv[1];

    // TODO make configurable
    // core_property_trace_log_enable(true);
    // core_property_node_trace_log_enable(true);

    _bootstrap_options(argc, argv);
    _bootstrap_config(config_path, &config);

    inject_main(&config);

    return 0;


    // inject 
    // general configuration stuff
    // --config (-c) inject-09.xml
    // override any parameters with key-value params
    // --param (-p) logger.level=asdf
    
    // some shortcuts for params commonly used
    // --loglevel (-l)
    // --logfile (-y)
    // --remotedebugger (-r)
    // --debugger (-d)
    // --configslog (-s)
    
    // -- hook.dll... app.exe [hooks options...]

    // TODO
    // - options
    // - load config
    // - apply overrides to config, hook dlls are added
}