#ifndef MODULE_H
#define MODULE_H

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

typedef struct module module_t;

void module_load(const char *path, module_t **module);
HMODULE module_handle_get(const module_t *module);
const char *module_path_get(const module_t *module);
void *module_func_required_resolve(const module_t *module, const char *name);
void *module_func_optional_resolve(const module_t *module, const char *name);
void module_func_required_verify(
    const module_t *module, void *func, const char *name);
void module_func_optional_verify(
    const module_t *module, void *func, const char *name);
void module_func_pre_invoke_log(const module_t *module, const char *name);
void module_func_post_invoke_log(const module_t *module, const char *name);
void module_free(module_t **module);

#endif