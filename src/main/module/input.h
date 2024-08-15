#ifndef MODULE_INPUT_H
#define MODULE_INPUT_H

#include <stdbool.h>
#include <stdint.h>

#include "api/core/log.h"
#include "api/core/thread.h"

#include "api/input.h"

typedef struct module_input module_input_t;

void module_input_load(const char *path, module_input_t **module);
void module_input_free(module_input_t **module);

void module_input_core_log_api_set(
    const module_input_t *module, const bt_core_log_api_t *api);
void module_input_core_thread_api_set(
    const module_input_t *module, const bt_core_thread_api_t *api);

void module_input_api_get(const module_input_t *module, bt_input_api_t *api);

#endif