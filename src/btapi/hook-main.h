#ifndef BTAPI_HOOK_MAIN_H
#define BTAPI_HOOK_MAIN_H

#include <windows.h>

#include "property.h"

typedef bool (*btapi_hook_main_init_t)(HMODULE game_module, struct property_node *property_node_config);
typedef void (*btapi_hook_main_fini_t)();

// game module reference, either the exe or dll. allow for further targeted hooking/patching
// remark: you can't own the memory of the property_node config. whatever you need form that, make sure to copy the data and not just reference it.
// there is no guarantee the data is not free'd/gone after this call returns
// use the property api to iterate the data and parse it into your own custom configuration struct
// it is advised to also validate all parameters
// if no configuration was provided upon loading, the config_node contains an empty root node
bool btapi_hook_main_init(HMODULE game_module, struct property_node *property_node_config);

void btapi_hook_main_fini();

#endif