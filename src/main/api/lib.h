#ifndef API_LIB_H
#define API_LIB_H

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

typedef struct api_lib api_lib_t;

void api_lib_load(const char *path, api_lib_t **lib);
HMODULE api_lib_module_get(const api_lib_t *lib);
const char *api_lib_path_get(const api_lib_t *lib);
void *api_lib_func_resolve(const api_lib_t *lib, const char *name, uint8_t version);
void *api_lib_func_optional_resolve(const api_lib_t *lib, const char *name, uint8_t version);
void api_lib_func_pre_invoke_log(const api_lib_t *lib, const char *name);
void api_lib_func_post_invoke_log(const api_lib_t *lib, const char *name);
void api_lib_free(api_lib_t **lib);

#endif