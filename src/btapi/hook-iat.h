#ifndef BTAPI_HOOK_IAT_H
#define BTAPI_HOOK_IAT_H

#include "property.h"

typedef const char* (*btapi_hook_iat_dll_name_get_t)();

// name of the dll to iat hook
const char *btapi_hook_iat_dll_name_get();

#endif