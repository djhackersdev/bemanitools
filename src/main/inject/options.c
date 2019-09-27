#include <stdio.h>
#include <stdlib.h>

#include "inject/options.h"

#include "util/defs.h"

void options_init(struct options *options)
{
    options->debug = false;
    options->remote_debugger = false;
    memset(options->log_file, 0, sizeof(options->log_file));
}

bool options_read_cmdline(struct options *options, int argc, char **argv)
{
    for (int i = 1 ; i < argc ; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'D':
                    options->debug = true;

                    break;

                case 'Y':
                    if (i + 1 >= argc) {
                        return false;
                    }

                    strcpy(options->log_file, argv[++i]);

                    break;

                case 'R':
                    options->remote_debugger = true;

                    break;

                default:
                    break;
            }
        }
    }

    return true;
}

void options_print_usage(void)
{
    fprintf(stderr,
"inject build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV) "\n"
"Usage: inject hook.dll... app.exe [hooks options...]\n"
"You can specify one or multiple hook.dll files, e.g. inject.exe "
"hook1.dll hook2.dll app.exe"
"\n"
"       The following options can be specified after the exe path:\n"
"\n"
"       -D Enable debugging output\n"
"       -R Halt the injected process until a debugger is attached\n"
"       -Y [filename]   Log to a file in addition to the console\n"
    );
}
