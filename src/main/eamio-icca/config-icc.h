#ifndef EAMIO_ICCA_CONFIG_ICC_H
#define EAMIO_ICCA_CONFIG_ICC_H

#include <windows.h>

#include "cconfig/cconfig.h"

struct icc_config {
    char port[64];
    int32_t baud;
};

void eamio_icca_config_icc_init(struct cconfig *config);

void eamio_icca_config_icc_get(
    struct icc_config *config_icc, struct cconfig *config);

#endif
