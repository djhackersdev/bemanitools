#ifndef PROCMON_CONFIG_H
#define PROCMON_CONFIG_H

#include <stdint.h>

#include "imports/avs.h"

struct procmon_config {
    uint32_t version;

    bool file_monitor_enable;
    bool module_monitor_enable;
    bool thread_monitor_enable;
};

void procmon_config_init(struct procmon_config *config);

void procmon_config_load(
    struct property_node *property_node, struct procmon_config *config);

void procmon_config_fini(struct procmon_config *config);

#endif