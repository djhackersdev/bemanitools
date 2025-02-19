#include <string.h>

#include "d3d9-monitor-check/cmdline.h"
#include "d3d9-monitor-check/interactive.h"
#include "d3d9-monitor-check/print-console.h"

static void _print_synopsis()
{
    printfln_err("D3D9 monitor check tool");
    printfln_err("");
    printfln_err("A versatile tool for running various (music game relevant) monitor tests using the D3D9 rendering API");
    printfln_err("");
    printfln_err("Available commands:");
    printfln_err("  cmdline: Run the tool in command line mode");
    printfln_err("  interactive: Run the tool in interactive mode");
}

int main(int argc, char **argv)
{
    const char *command;
    bool success;

    if (argc < 2) {
        _print_synopsis();
        printfln_err("ERROR: Insufficient arguments");
        return 1;
    }

    command = argv[1];

    if (!strcmp(command, "cmdline")) {
        success = cmdline_main(argc - 2, argv + 2);
    } else if (!strcmp(command, "interactive")) {
        success = interactive_main(argc - 2, argv + 2);
    } else {
        _print_synopsis();
        success = false;
    }

    return success ? 0 : 1;
}
