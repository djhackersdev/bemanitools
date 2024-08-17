#ifndef MODULE_CONFIGURE_H
#define MODULE_CONFIGURE_H

#include "api/core/config.h"

typedef bool (*bt_module_configure_do_t)(const bt_core_config_t *config);

#endif