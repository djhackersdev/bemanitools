#ifndef CONFIG_GAMETYPE_H
#define CONFIG_GAMETYPE_H

#include <windows.h>

#include "config/schema.h"

const struct schema *game_type_from_str(const char *name);
const struct schema *game_type_from_dialog(HINSTANCE inst);

#endif
