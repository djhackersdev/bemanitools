#include "jbhook3/options.h"

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iface-core/log.h"

#include "util/cmdline.h"
#include "util/defs.h"
#include "util/hex.h"
#include "util/str.h"

void options_init_from_cmdline(struct options *options)
{
    int argc;
    char **argv;
    bool ok;

    args_recover(&argc, &argv);

    options_init(options);

    ok = options_read_cmdline(options, argc, (const char **) argv);

    args_free(argc, argv);

    if (!ok) {
        exit(0);
    }
}

void options_init(struct options *options)
{
    options->windowed = false;
    options->disable_p4ioemu = false;
    options->disable_cardemu = false;
    options->disable_adapteremu = false;
    options->show_cursor = false;
}

bool options_read_cmdline(struct options *options, int argc, const char **argv)
{
    int i;

    for (i = 0; i < argc; i++) {
        if (argv[i][0] != '-') {
            continue;
        }

        switch (argv[i][1]) {
            case 'h': {
                options_print_usage();
                return false;
            }

            case 'w': {
                options->windowed = true;
                break;
            }

            case 'a': {
                options->disable_adapteremu = true;
                break;
            }

            case 'c': {
                options->disable_cardemu = true;
                break;
            }

            case 'p': {
                options->disable_p4ioemu = true;
                break;
            }

            case 's': {
                options->show_cursor = true;
                break;
            }
        }
    }

    return true;
}

void options_print_usage(void)
{
    OutputDebugStringA("jbhook for jubeat, build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(
        GITREV) "\n"
                "Usage: launcher.exe -K jbhook3.dll [game exec] <options> \n"
                "\n"
                "       The following options can be specified after the game exec path:\n"
                "\n"
                "       -h                  Print this usage message\n"
                "       -w                  Run the game windowed\n"
                "       -a                  Disable adapter hook\n"
                "       -c                  Disable card emulation (e.g. when running on a "
                "real cab)\n"
                "       -p                  Disable p4io emulation (e.g. when running on a "
                "real cab or on a bare "
                "p4io)\n"
                "       -s                  Show mouse cursor on game window\n");
}

void options_fini(struct options *options)
{
}
