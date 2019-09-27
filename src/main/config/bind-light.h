#ifndef CONFIG_BIND_LIGHT_H
#define CONFIG_BIND_LIGHT_H

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

#include "config/schema.h"

#include "geninput/mapper.h"

bool bind_light(HINSTANCE inst, HWND hwnd, const struct schema *schema,
        const struct mapped_light *ml, uint8_t *game_light, bool *bound);

#endif
