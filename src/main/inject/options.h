#ifndef INJECT_OPTIONS_H
#define INJECT_OPTIONS_H

#include <stdbool.h>
#include <stddef.h>
#include <windows.h>

#include "util/array.h"

struct options {
    bool debug;
    bool remote_debugger;
    char log_file[MAX_PATH];
};

void options_init(struct options *options);
bool options_read_cmdline(struct options *options, int argc, char **argv);
void options_print_usage(void);

#endif
