#ifndef HOOKLIB_APP_H
#define HOOKLIB_APP_H

#include <stdbool.h>

#include "imports/avs.h"
#include "imports/eapki.h"

void app_hook_init(dll_entry_init_t init, dll_entry_main_t main_);
bool app_hook_invoke_init(char *sidcode, struct property_node *config);
bool app_hook_invoke_main(void);

#endif
