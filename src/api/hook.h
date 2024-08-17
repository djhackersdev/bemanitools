#ifndef BT_API_HOOK_H
#define BT_API_HOOK_H

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "api/core/config.h"

typedef void (*bt_hook_iat_dll_name_get_t)(char *buffer, size_t size);
typedef bool (*bt_hook_pre_avs_init_t)(const bt_core_config_t *config);
// game module reference, either the exe or dll. allow for further targeted hooking/patching
// remark: you can't own the memory of the property_node config. whatever you need form that, make sure to copy the data and not just reference it.
// there is no guarantee the data is not free'd/gone after this call returns
// use the property api to iterate the data and parse it into your own custom configuration struct
// it is advised to also validate all parameters
// if no configuration was provided upon loading, the config_node contains an empty root node
typedef bool (*bt_hook_main_init_t)(HMODULE game_module, const bt_core_config_t *config);
typedef void (*bt_hook_main_fini_t)();

typedef struct bt_hook_api {
    uint16_t version;

    struct {
        // Optional
        bt_hook_iat_dll_name_get_t iat_dll_name_get;
        bt_hook_pre_avs_init_t pre_avs_init;
        bt_hook_main_init_t main_init;
        bt_hook_main_fini_t main_fini;
    } v1;
} bt_hook_api_t;

#endif