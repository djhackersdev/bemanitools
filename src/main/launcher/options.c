#define LOG_MODULE "options"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "imports/avs.h"

#include "launcher/options.h"

#include "util/mem.h"
#include "util/str.h"

void options_init(struct options *options)
{
    memset(options, 0, sizeof(struct options));

    array_init(&options->hooks.paths);

    options->log.level = CORE_LOG_BT_LOG_LEVEL_INFO;
}

bool options_read_cmdline(struct options *options, int argc, const char **argv)
{
    bool got_launcher_config;

    got_launcher_config = false;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'B':
                    if (i + 1 >= argc) {
                        return false;
                    }

                    options->bootstrap.config_path = argv[++i];

                    break;

                case 'Z':
                    if (i + 1 >= argc) {
                        return false;
                    }

                    options->bootstrap.selector = argv[++i];

                    break;

                case 'P':
                    if (i + 1 >= argc) {
                        return false;
                    }

                    options->eamuse.pcbid = argv[++i];

                    break;

                case 'R':
                    if (i + 1 >= argc) {
                        return false;
                    }

                    options->eamuse.softid = argv[++i];

                    break;

                case 'H':
                    if (i + 1 >= argc) {
                        return false;
                    }

                    *array_append(const char *, &options->hooks.paths) =
                        argv[++i];

                    break;

                case 'L':
                    if (i + 1 >= argc) {
                        return false;
                    }

                    long tmp = strtol(argv[++i], NULL, 0);

                    if (tmp < CORE_LOG_BT_LOG_LEVEL_OFF ||
                        tmp > CORE_LOG_BT_LOG_LEVEL_MISC) {
                        return false;
                    }

                    options->log.level = (enum core_log_bt_log_level) tmp;

                    break;

                case 'Y':
                    if (i + 1 >= argc) {
                        return false;
                    }

                    options->log.file_path = argv[++i];

                    break;

                case 'C':
                    options->debug.log_property_configs = true;

                    break;

                case 'S':
                    if (i + 1 >= argc) {
                        return false;
                    }

                    options->eamuse.service_url = argv[++i];

                    break;

                case 'U':
                    if (i + 1 >= argc) {
                        return false;
                    }

                    options->eamuse.urlslash = xmalloc(sizeof(bool));

                    const char *urlslash_value = argv[++i];

                    *(options->eamuse.urlslash) = false;

                    if (_stricmp(urlslash_value, "1") == 0) {
                        *(options->eamuse.urlslash) = true;
                    }
                    if (_stricmp(urlslash_value, "true") == 0) {
                        *(options->eamuse.urlslash) = true;
                    }

                    break;

                case 'D':
                    options->debug.remote_debugger = true;

                    break;

                default:
                    break;
            }
        } else {
            if (!got_launcher_config) {
                options->launcher.config_path = argv[i];
                got_launcher_config = true;
            }
        }
    }

    return got_launcher_config;
}

void options_print_usage(void)
{
    fprintf(
        stderr,
        "Usage:\n"
        "  launcher.exe [launcher options as overrides...] <path launcher.xml> "
        "[further options, e.g. for hook libraries to pick up...]\n"
        "\n"
        "       The following options can be specified before the launcher.xml "
        "configuration file:\n"
        "\n"
        "  Bootstrap\n"
        "       -B [filename]   Bootstrap configuration file\n"
        "       -Z [selector]   Bootstrap selector used in configuration\n"
        "\n"
        "  Eamuse\n"
        "       -P [pcbid]      Specify PCBID\n"
        "       -R [softid]     Specify Soft ID\n"
        "       -S [url]        Specify service url\n"
        "       -U [0/1]        Specify url_slash enabled/disabled\n"
        "\n"
        "  Hook\n"
        "       -H [filename]   Load hook DLL (can be specified multiple "
        "\n"
        "  Logging\n"
        "       -L [0/1/2/3]    Log level for both console and file with "
        "increasing verbosity (0 = fatal, 1 = warn, 2 = info, 3 = misc)\n"
        "       -Y [filename]   Log to a file in addition to the console\n"
        "\n"
        "  Debug\n"
        "       -D              Halt the launcher before bootstrapping AVS "
        "until a remote debugger is attached\n"
        "       -C              Log all loaded and final (property) "
        "configuration that launcher uses for bootstrapping. IMPORTANT: DO NOT "
        "ENABLE unless you know what you are doing. This prints sensitive data "
        "and credentials to the console and logfile. BE CAUTIOUS not to share "
        "this information before redaction.");
}

void options_fini(struct options *options)
{
    array_fini(&options->hooks.paths);

    if (options->eamuse.urlslash) {
        free(options->eamuse.urlslash);
    }
}
