#ifndef DDRIO_CONFIG_P4IO_MDXF_M
#define DDRIO_CONFIG_P4IO_MDXF_M

#include <windows.h>

#include "cconfig/cconfig.h"

struct p4io_mdxf_config {
    char port[64];
    int32_t baud;
};

void ddrio_config_p4io_mdxf_init(struct cconfig *config);

void ddrio_config_p4io_mdxf_get(
    struct p4io_mdxf_config *config_p4io_mdxf, struct cconfig *config);

#endif
