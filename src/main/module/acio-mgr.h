#ifndef MODULE_ACIO_MGR_H
#define MODULE_ACIO_MGR_H

#include <stdbool.h>
#include <stdint.h>

#include "api/acio/mgr.h"
#include "api/core/log.h"

typedef struct module_acio_mgr module_acio_mgr_t;

void module_acio_mgr_load(const char *path, module_acio_mgr_t **module);
void module_acio_mgr_free(module_acio_mgr_t **module);

void module_acio_mgr_core_log_api_set(
    const module_acio_mgr_t *module, const bt_core_log_api_t *api);

void module_acio_mgr_api_get(
    const module_acio_mgr_t *module, bt_acio_mgr_api_t *api);

#endif