#ifndef BTAPI_HOOK_BEFORE_AVS_H
#define BTAPI_HOOK_BEFORE_AVS_H

#include "btapi/config.h"

typedef bool (*btapi_hook_before_avs_init_t)(const btapi_config_t *config);

bool btapi_hook_before_avs_init(const btapi_config_t *config);

#endif