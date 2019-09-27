#define LOG_MODULE "cconfig-cmd"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "cconfig/cmd.h"

#include "util/hex.h"
#include "util/log.h"
#include "util/str.h"

static void cconfig_cmd_usage_print(enum cconfig_cmd_usage_out output, 
        const char* fmt, ...)
{
    char buffer[32768];
    va_list ap;

    va_start(ap, fmt);

    switch (output) {
        case CCONFIG_CMD_USAGE_OUT_STDOUT:
            vfprintf(stdout, fmt, ap);
            break;

        case CCONFIG_CMD_USAGE_OUT_STDERR:
            vfprintf(stderr, fmt, ap);
            break;

        case CCONFIG_CMD_USAGE_OUT_DBG:
            _vsnprintf(buffer, sizeof(buffer), fmt, ap);
            OutputDebugString(buffer);
            break;

        case CCONFIG_CMD_USAGE_OUT_LOG:
            _vsnprintf(buffer, sizeof(buffer), fmt, ap);
            log_info("%s", buffer);
            break;

        default:
            log_assert(false);
            break;
    }

    va_end(ap);
}

bool cconfig_cmd_parse(struct cconfig* config, const char* key_ident, int argc, 
        char** argv, bool add_params_if_absent)
{
    bool no_error;
    struct cconfig_entry* entry;
    char* tmp;
    char* cur_tok;
    int ntok;
    char* toks[2];

    no_error = true;

    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], key_ident)) {
            if (i + 1 >= argc) {
                no_error = false;
                break;
            }

            /* Not another key ident is following */
            if (!strcmp(argv[i + 1], key_ident)) {
                no_error = false;
                break;
            }

            ++i;

            tmp = str_dup(argv[i]);
            ntok = 0;

            cur_tok = strtok(tmp, "=");

            while (cur_tok != NULL) {
                toks[ntok] = cur_tok;
                ntok++;
                cur_tok = strtok(NULL, "=");

                if (ntok == 2) {
                    break;
                }
            }

            if (ntok != 2) {
                /* Tokenizing key=value parameter error */
                log_warning("Parsing parameter '%s' failed, ignore", argv[i]);

                free(tmp);
                no_error = false;
                continue;
            }

            log_misc("Key: %s, Value: %s", toks[0], toks[1]);

            entry = cconfig_get(config, toks[0]);

            if (entry || add_params_if_absent) {
                cconfig_set2(config, toks[0], toks[1]);
            } else {
                /* Ignore cmd params that are not found in config */
                log_warning("Could not find cmd parameter with key '%s' in "
                    "config, ignored", toks[0]);
            }

            free(tmp);
        }
    }

    return no_error;
}

void cconfig_cmd_print_usage(struct cconfig* config, const char* usage_header,
        enum cconfig_cmd_usage_out output)
{
    cconfig_cmd_usage_print(output, "%s\n", usage_header);

    for (uint32_t i = 0; i < config->nentries; i++) {
        cconfig_cmd_usage_print(output,
            "    %s: %s\n"
            "      default: %s\n",
            config->entries[i].key,
            config->entries[i].desc,
            config->entries[i].value);
    }
}