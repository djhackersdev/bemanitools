#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "imports/avs.h"

#include "launcher/options.h"

#include "util/array.h"
#include "util/log.h"
#include "util/str.h"

#define DEFAULT_HEAP_SIZE 16777216

void options_init(struct options *options)
{
    options->std_heap_size = DEFAULT_HEAP_SIZE;
    options->avs_heap_size = DEFAULT_HEAP_SIZE;
    options->app_config_path = "prop/app-config.xml";
    options->avs_config_path = "prop/avs-config.xml";
    options->ea3_config_path = "prop/ea3-config.xml";
    options->ea3_ident_path = "prop/ea3-ident.xml";
    options->softid = NULL;
    options->pcbid = NULL;
    options->module = NULL;
    options->logfile = NULL;
    options->remote_debugger = false;
    array_init(&options->hook_dlls);
    array_init(&options->before_hook_dlls);
    array_init(&options->iat_hook_dlls);

    options->override_service = NULL;
    options->override_urlslash_enabled = false;
    options->override_urlslash_value = false;
}

bool options_read_cmdline(struct options *options, int argc, const char **argv)
{
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'A':
                    if (i + 1 >= argc) {
                        return false;
                    }

                    options->app_config_path = argv[++i];

                    break;

                case 'E':
                    if (i + 1 >= argc) {
                        return false;
                    }

                    options->ea3_config_path = argv[++i];

                    break;

                case 'V':
                    if (i + 1 >= argc) {
                        return false;
                    }

                    options->avs_config_path = argv[++i];

                    break;

                case 'P':
                    if (i + 1 >= argc) {
                        return false;
                    }

                    options->pcbid = argv[++i];

                    break;

                case 'R':
                    if (i + 1 >= argc) {
                        return false;
                    }

                    options->softid = argv[++i];

                    break;

                case 'H':
                    if (i + 1 >= argc) {
                        return false;
                    }

                    options->avs_heap_size =
                        (size_t) strtol(argv[++i], NULL, 0);

                    if (options->avs_heap_size == 0) {
                        return false;
                    }

                    break;

#ifdef AVS_HAS_STD_HEAP
                case 'T':
                    if (i + 1 >= argc) {
                        return false;
                    }

                    options->std_heap_size =
                        (size_t) strtol(argv[++i], NULL, 0);

                    if (options->std_heap_size == 0) {
                        return false;
                    }

                    break;
#endif

                case 'K':
                    if (i + 1 >= argc) {
                        return false;
                    }

                    *array_append(const char *, &options->hook_dlls) =
                        argv[++i];

                    break;

                case 'B':
                    if (i + 1 >= argc) {
                        return false;
                    }

                    *array_append(const char *, &options->before_hook_dlls) =
                        argv[++i];

                    break;

                case 'I': {
                    if (i + 1 >= argc) {
                        return false;
                    }

                    const char *dll = argv[++i];
                    log_assert(strstr(dll, "=") != NULL);

                    *array_append(const char *, &options->iat_hook_dlls) = dll;

                    break;
                }

                case 'Y':
                    if (i + 1 >= argc) {
                        return false;
                    }

                    options->logfile = argv[++i];

                    break;

                case 'S':
                    if (i + 1 >= argc) {
                        return false;
                    }

                    options->override_service = argv[++i];

                    break;

                case 'U':
                    if (i + 1 >= argc) {
                        return false;
                    }

                    options->override_urlslash_enabled = true;

                    const char *urlslash_value = argv[++i];

                    options->override_urlslash_value = false;
                    if (_stricmp(urlslash_value, "1") == 0) {
                        options->override_urlslash_value = true;
                    }
                    if (_stricmp(urlslash_value, "true") == 0) {
                        options->override_urlslash_value = true;
                    }

                    break;

                case 'D':
                    options->remote_debugger = true;

                    break;

                default:
                    break;
            }
        } else {
            if (!options->module) {
                options->module = argv[i];
            }
        }
    }

    if (options->module) {
        return true;
    } else {
        return false;
    }
}

void options_print_usage(void)
{
    fprintf(
        stderr,
        "Usage: launcher.exe [launcher options...] <app.dll> [hooks "
        "options...] \n"
        "\n"
        "       The following options can be specified before the app DLL "
        "path:\n"
        "\n"
        "       -A [filename]   App configuration file (default: "
        "prop/app-config.xml)\n"
        "       -V [filename]   AVS configuration file (default: "
        "prop/avs-config.xml)\n"
        "       -E [filename]   ea3 configuration file (default: "
        "prop/ea3-config.xml)\n"
        "       -H [bytes]      AVS heap size (default: 16777216)\n"
#ifdef AVS_HAS_STD_HEAP
        "       -T [bytes]      'std' heap size (default 16777216)\n"
#endif
        "       -P [pcbid]      Specify PCBID (default: use ea3 config)\n"
        "       -R [pcbid]      Specify Soft ID (default: use ea3 config)\n"
        "       -S [url]        Specify service url (default: use ea3 config)\n"
        "       -U [0/1]        Specify url_slash (default: use ea3 config)\n"
        "       -K [filename]   Load hook DLL (can be specified multiple "
        "times)\n"
        "       -B [filename]   Load pre-hook DLL loaded before avs boot "
        "(can be specified multiple times)\n"
        "       -I [filename]   Load pre-hook DLL that overrides IAT reference "
        "before execution"
        "(can be specified multiple times)\n"
        "       -Y [filename]   Log to a file in addition to the console\n"
        "       -D              Halt the launcher before bootstrapping AVS "
        "until a"
        " remote debugger is attached\n");
}

void options_fini(struct options *options)
{
    array_fini(&options->hook_dlls);
    array_fini(&options->before_hook_dlls);
    array_fini(&options->iat_hook_dlls);
}
