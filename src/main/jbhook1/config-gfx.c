#include <string.h>

#include "core/config-ext.h"

#include "jbhook1/config-gfx.h"

void jbhook1_config_gfx_get(
    const bt_core_config_t *config,
    jbhook1_config_gfx_t *config_out)
{
    bt_core_config_ext_bool_get(config, "gfx/windowed", &config_out->windowed);
    bt_core_config_ext_bool_get(config, "gfx/vertical", &config_out->vertical);
}
