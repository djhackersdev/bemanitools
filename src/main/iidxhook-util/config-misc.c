#include "core/config-ext.h"

#include "iidxhook-util/config-misc.h"

void iidxhook_util_config_misc_get(
    const bt_core_config_t *config,
    iidxhook_config_misc_t *config_misc)
{
    bt_core_config_ext_bool_get(config, "misc/disable_clock_set", &config_misc->disable_clock_set);
    bt_core_config_ext_bool_get(config, "misc/rteffect_stub", &config_misc->rteffect_stub);
    bt_core_config_ext_str_get(config, "misc/settings_path", config_misc->settings_path, sizeof(config_misc->settings_path));
}
