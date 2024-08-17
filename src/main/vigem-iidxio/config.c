#include "core/config-ext.h"

#include "vigem-iidxio/config.h"

void vigem_iidxio_config_get(const bt_core_config_t *config, vigem_iidxio_config_t *config_out)
{
    bt_core_config_ext_bool_get(config, "tt/anlog/relative", &config_out->tt.analog.relative);
    bt_core_config_ext_s32_get(config, "tt/anlog/relative_sensitivity", &config_out->tt.analog.relative_sensitivity);
    bt_core_config_ext_s32_get(config, "tt/button/debounce", &config_out->tt.button.debounce);
    bt_core_config_ext_s32_get(config, "tt/button/threshold", &config_out->tt.button.threshold);
    bt_core_config_ext_bool_get(config, "tt/debug_output", &config_out->tt.debug_output);
    bt_core_config_ext_bool_get(config, "cab_light/enable_keylight", &config_out->cab_light.enable_keylight);
    bt_core_config_ext_s32_get(config, "cab_light/light_mode", &config_out->cab_light.light_mode);
    bt_core_config_ext_str_get(config, "cab_light/text_16seg", config_out->cab_light.text_16seg, sizeof(config_out->cab_light.text_16seg));
    bt_core_config_ext_s32_get(config, "cab_light/text_scroll_cycle_time_ms", &config_out->cab_light.text_scroll_cycle_time_ms);
}