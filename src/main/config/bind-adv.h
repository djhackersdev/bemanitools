#ifndef CONFIG_BIND_ADV_H
#define CONFIG_BIND_ADV_H

#include <windows.h>

#include <stdbool.h>

#include "config/schema.h"

#include "geninput/input-config.h"

bool bind_adv(
    HINSTANCE inst, HWND hwnd, struct mapped_action *ma, bool was_valid);

#endif
