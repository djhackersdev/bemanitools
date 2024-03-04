#ifndef PROCMON_PROCMON_H
#define PROCMON_PROCMON_H

#include "procmon/config.h"

void procmon_init(const struct procmon_config *config);

void procmon_fini();

#endif