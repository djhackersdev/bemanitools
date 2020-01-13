#ifndef SDVXHOOK2_CN_CONFIG_H
#define SDVXHOOK2_CN_CONFIG_H

#include <windows.h>

#include "cconfig/cconfig.h"

struct sdvxhook2_cn_config {
    bool disable_io_emu;
    char unis_path[256];
};

void sdvxhook2_cn_config_init(struct cconfig *config);

void sdvxhook2_cn_config_get(
    struct sdvxhook2_cn_config *cn_config, struct cconfig *config);

#endif