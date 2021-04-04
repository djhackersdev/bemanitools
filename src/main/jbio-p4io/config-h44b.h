#ifndef JBIO_CONFIG_H44B_H
#define JBIO_CONFIG_H44B_H

#include <windows.h>

#include "cconfig/cconfig.h"

struct h44b_config {
    char port[64];
    int32_t baud;
};

void jbio_config_h44b_init(struct cconfig *config);

void jbio_config_h44b_get(
    struct h44b_config *config_h44b, struct cconfig *config);

#endif
