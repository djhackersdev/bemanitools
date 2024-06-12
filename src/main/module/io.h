#ifndef MODULE_IO_H
#define MODULE_IO_H

#include <stdbool.h>
#include <stdint.h>

#include "api/core/config.h"
#include "api/core/log.h"
#include "api/core/thread.h"

typedef struct module_io module_io_t;

void module_io_load(
    const char *path, const char *api_get_func_name, module_io_t **module);
void module_io_free(module_io_t **module);

void module_io_core_config_api_set(
    const module_io_t *module, const bt_core_config_api_t *api);
void module_io_core_log_api_set(
    const module_io_t *module, const bt_core_log_api_t *api);
void module_io_core_thread_api_set(
    const module_io_t *module, const bt_core_thread_api_t *api);

void module_io_api_get(const module_io_t *module, void *api);

#endif