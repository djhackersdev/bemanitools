#ifndef NVGPU_CONFIG_NVAPI_MODULE_H
#define NVGPU_CONFIG_NVAPI_MODULE_H

#include "nv/api.h"

typedef struct nv_module nv_module_t;

void nv_module_load(nv_module_t **module);
void nv_module_free(nv_module_t **module);

void nv_module_api_get(const nv_module_t *module, nv_api_t *api);

#endif