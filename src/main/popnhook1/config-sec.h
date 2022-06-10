#ifndef POPNHOOK1_CONFIG_SEC_H
#define POPNHOOK1_CONFIG_SEC_H

#include "cconfig/cconfig.h"

#include "security/mcode.h"

struct popnhook1_config_sec {
    struct security_mcode black_plug_mcode;
};

void popnhook1_config_sec_init(struct cconfig *config);

void popnhook1_config_sec_get(
    struct popnhook1_config_sec *config_sec, struct cconfig *config);

#endif