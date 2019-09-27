#include <stdbool.h>

#include "cconfig/cconfig.h"

enum cconfig_conf_error {
    CCONFIG_CONF_SUCCESS = 0,
    CCONFIG_CONF_ERROR_NO_SUCH_FILE = 1,
    CCONFIG_CONF_ERROR_FILE_CORRUPTED = 2,
    CCONFIG_CONF_ERROR_PARSING = 3,
};

enum cconfig_conf_error cconfig_conf_load_from_file(struct cconfig* config,
        const char* path, bool add_params_if_absent);

enum cconfig_conf_error cconfig_conf_save_to_file(struct cconfig* config,
        const char* path);