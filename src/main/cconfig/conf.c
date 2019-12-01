#define LOG_MODULE "cconfig-conf"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cconfig/conf.h"

#include "util/fs.h"
#include "util/log.h"
#include "util/str.h"

enum cconfig_conf_error cconfig_conf_load_from_file(
    struct cconfig *config, const char *path, bool add_params_if_absent)
{
    char *pos_lines;
    char *pos_key_val;
    char *ctx_lines;
    char *ctx_key_val;
    char *data;
    size_t len;
    bool use_crlf;

    if (!file_load(path, (void **) &data, &len, true)) {
        /* If file does not exist, create one with default configuration
           values */
        if (path_exists(path)) {
            return CCONFIG_CONF_ERROR_FILE_CORRUPTED;
        } else {
            return CCONFIG_CONF_ERROR_NO_SUCH_FILE;
        }
    }

    use_crlf = false;
    if (strstr(data, "\r\n")) {
        use_crlf = true;
    }
    pos_lines = strtok_r(data, use_crlf ? "\r\n" : "\n", &ctx_lines);

    while (pos_lines != NULL) {
        char *pos_line_dup;
        char *key = NULL;
        char *val = NULL;
        int cnt = 0;
        struct cconfig_entry *entry;

        /* ignore comments and empty lines */
        if (strlen(pos_lines) > 0 && pos_lines[0] != '#') {
            pos_line_dup = str_dup(pos_lines);
            pos_key_val = strtok_r(pos_line_dup, "=", &ctx_key_val);

            log_misc("Line: %s", pos_lines);

            while (pos_key_val != NULL) {
                if (cnt == 0) {
                    key = pos_key_val;
                } else if (cnt == 1) {
                    val = pos_key_val;
                }

                pos_key_val = strtok_r(NULL, "=", &ctx_key_val);
                cnt++;
            }

            /* Key requiured, value can be NULL */
            if (cnt != 1 && cnt != 2) {
                log_warning(
                    "Invalid options line %s in options file %s",
                    pos_lines,
                    path);
                free(pos_line_dup);
                free(data);
                return CCONFIG_CONF_ERROR_PARSING;
            }

            /* NULL not allowed but empty string */
            if (!val) {
                val = "";
            }

            log_misc("Key: %s, Value: %s", key, val);

            entry = cconfig_get(config, key);

            if (entry || add_params_if_absent) {
                cconfig_set2(config, key, val);
            } else {
                /* Ignore cmd params that are not found in config */
                log_warning(
                    "Could not find parameter with key '%s' in "
                    "config, ignored",
                    key);
            }

            free(pos_line_dup);
        }

        pos_lines = strtok_r(NULL, use_crlf ? "\r\n" : "\n", &ctx_lines);
    }

    free(data);

    return CCONFIG_CONF_SUCCESS;
}

enum cconfig_conf_error
cconfig_conf_save_to_file(struct cconfig *config, const char *path)
{
    FILE *file;

    file = fopen(path, "wb+");

    if (file == NULL) {
        return CCONFIG_CONF_ERROR_NO_SUCH_FILE;
    }

    for (uint32_t i = 0; i < config->nentries; i++) {
        fprintf(file, "# %s\n", config->entries[i].desc);
        fprintf(
            file,
            "%s=%s\n\n",
            config->entries[i].key,
            config->entries[i].value);
    }

    fclose(file);
    return CCONFIG_CONF_SUCCESS;
}