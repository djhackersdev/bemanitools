#include <stdlib.h>

#include "core/boot.h"

#include "launcher/launcher.h"
#include "launcher/options.h"

int main(int argc, const char **argv)
{
    struct options options;

    core_boot("launcher");

    options_init(&options);

    if (!options_read_cmdline(&options, argc, argv)) {
        options_print_usage();

        exit(EXIT_FAILURE);
    }

    launcher_main(&options);

    options_fini(&options);

    return EXIT_SUCCESS;
}