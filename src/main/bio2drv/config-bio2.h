#ifndef BIO2DRV_CONFIG_BIO2_H
#define BIO2DRV_CONFIG_BIO2_H

#include <windows.h>

#include "cconfig/cconfig.h"

struct bio2drv_config_bio2 {
    bool autodetect;
    char port[64];
    int32_t baud;
};

void bio2drv_config_bio2_init(struct cconfig *config);

void bio2drv_config_bio2_get(
    struct bio2drv_config_bio2 *config_bio2, struct cconfig *config);

#endif
