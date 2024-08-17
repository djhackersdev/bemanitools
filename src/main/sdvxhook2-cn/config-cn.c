#include "core/config-ext.h"

#include "sdvxhook2-cn/config-cn.h"

void sdvxhook2_cn_config_get(
    const bt_core_config_t *config, sdvxhook2_cn_config_t *config_out)
{
    bt_core_config_ext_bool_get(config, "io/disable_io_emu", &config_out->disable_io_emu);
    bt_core_config_ext_str_get(config, "cn/unis_path", config_out->unis_path, sizeof(config_out->unis_path));
}
