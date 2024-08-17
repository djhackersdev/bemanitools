#include <string.h>

#include "core/config-ext.h"

#include "ddrhook1/config-ddrhook1.h"

void ddrhook1_config_ddrhook1_get(
    const bt_core_config_t *config,
    ddrhook1_config_ddrhook1_t *config_ddrhook1)
{
    bt_core_config_ext_bool_get(config, "game/use_com4_emu", &config_ddrhook1->use_com4_emu);
    bt_core_config_ext_bool_get(config, "game/standard_def", &config_ddrhook1->standard_def);
    bt_core_config_ext_bool_get(config, "game/use_15khz", &config_ddrhook1->use_15khz);
    bt_core_config_ext_bool_get(config, "game/usbmem/enabled", &config_ddrhook1->usbmem_enabled);
    bt_core_config_ext_str_get(config, "game/usbmem/path_p1", config_ddrhook1->usbmem_path_p1, sizeof(config_ddrhook1->usbmem_path_p1));
    bt_core_config_ext_str_get(config, "game/usbmem/path_p2", config_ddrhook1->usbmem_path_p2, sizeof(config_ddrhook1->usbmem_path_p2));
}
