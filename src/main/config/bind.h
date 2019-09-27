#ifndef CONFIG_BIND_H
#define CONFIG_BIND_H

#include <windows.h>

#include <stdbool.h>

#include "config/schema.h"

#include "geninput/mapper.h"

bool bind_control(HINSTANCE inst, HWND hwnd, const struct action_def *action,
        struct mapped_action *ma);

#endif
