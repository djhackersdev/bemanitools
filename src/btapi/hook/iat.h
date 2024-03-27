#ifndef BTAPI_HOOK_IAT_H
#define BTAPI_HOOK_IAT_H

typedef const char *(*bt_hook_iat_dll_name_get_t)();

// name of the dll to iat hook
const char *bt_hook_iat_dll_name_get();

#endif