#ifndef PROCMON_PROCMON_H
#define PROCMON_PROCMON_H

#include "procmon/config.h"

void procmon_init(const procmon_config_t *config);

void procmon_fini();

#endif