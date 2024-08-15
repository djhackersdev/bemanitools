#ifndef INJECT_LOGGER_H
#define INJECT_LOGGER_H

#include "inject/logger-config.h"

void logger_init(const logger_config_t *config);

void logger_fini();

#endif