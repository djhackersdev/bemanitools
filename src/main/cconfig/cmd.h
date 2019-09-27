#ifndef CCONFIG_CMD_H
#define CCONFIG_CMD_H

#include <stdbool.h>

#include "cconfig/cconfig.h"

enum cconfig_cmd_usage_out {
    CCONFIG_CMD_USAGE_OUT_STDOUT,
    CCONFIG_CMD_USAGE_OUT_STDERR,
    CCONFIG_CMD_USAGE_OUT_DBG,
    CCONFIG_CMD_USAGE_OUT_LOG,
};

bool cconfig_cmd_parse(struct cconfig* config, const char* key_ident, int argc, 
        char** argv, bool add_params_if_absent);

void cconfig_cmd_print_usage(struct cconfig* config, const char* usage_header,
        enum cconfig_cmd_usage_out output);

#endif