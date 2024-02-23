#include "cconfig/cconfig-main.h"
#include "cconfig/cconfig-util.h"

#include "core/log.h"

#include "vigem-iidxio/config.h"

#define VIGEM_IIDXIO_CONFIG_TT_ANALOG_RELATIVE_KEY \
    "vigem.iidxio.tt.anlog.relative"
#define VIGEM_IIDXIO_CONFIG_TT_ANALOG_RELATIVE_SENSITIVITY_KEY \
    "vigem.iidxio.tt.anlog.relative_sensitivity"
#define VIGEM_IIDXIO_CONFIG_TT_BUTTON_DEBOUNCE_KEY \
    "vigem.iidxio.tt.button.debounce"
#define VIGEM_IIDXIO_CONFIG_TT_BUTTON_THRESHOLD_KEY \
    "vigem.iidxio.tt.button.threshold"
#define VIGEM_IIDXIO_CONFIG_TT_DEBUG_OUTPUT_KEY "vigem.iidxio.tt.debug_output"
#define VIGEM_IIDXIO_CONFIG_CAB_LIGHT_ENABLE_KEYLIGHT_KEY \
    "vigem.iidxio.cab_light.enable_keylight"
#define VIGEM_IIDXIO_CONFIG_CAB_LIGHT_LIGHT_MODE_KEY \
    "vigem.iidxio.cab_light.light_mode"
#define VIGEM_IIDXIO_CONFIG_CAB_LIGHT_TEXT_16SEG_KEY \
    "vigem.iidxio.cab_light.text_16seg"
#define VIGEM_IIDXIO_CONFIG_CAB_LIGHT_TEXT_SCROLL_CYCLE_TIME_MS_KEY \
    "vigem.iidxio.cab_light.text_scroll_cycle_time_ms"

#define VIGEM_IIDXIO_CONFIG_DEFAULT_TT_ANALOG_RELATIVE_VALUE false
#define VIGEM_IIDXIO_CONFIG_DEFAULT_TT_ANALOG_RELATIVE_SENSITIVITY_VALUE 1024
#define VIGEM_IIDXIO_CONFIG_DEFAULT_TT_BUTTON_DEBOUNCE_VALUE 20
#define VIGEM_IIDXIO_CONFIG_DEFAULT_TT_BUTTON_THRESHOLD_VALUE 2
#define VIGEM_IIDXIO_CONFIG_DEFAULT_TT_DEBUG_OUTPUT_VALUE false
#define VIGEM_IIDXIO_CONFIG_DEFAULT_CAB_LIGHT_ENABLE_KEYLIGHT_VALUE true
#define VIGEM_IIDXIO_CONFIG_DEFAULT_CAB_LIGHT_LIGHT_MODE_VALUE 0
#define VIGEM_IIDXIO_CONFIG_DEFAULT_CAB_LIGHT_TEXT_16SEG_VALUE ""
#define VIGEM_IIDXIO_CONFIG_DEFAULT_CAB_LIGHT_TEXT_SCROLL_CYCLE_TIME_MS_VALUE \
    500

static void _vigem_iidxio_config_init(struct cconfig *config)
{
    cconfig_util_set_bool(
        config,
        VIGEM_IIDXIO_CONFIG_TT_ANALOG_RELATIVE_KEY,
        VIGEM_IIDXIO_CONFIG_DEFAULT_TT_ANALOG_RELATIVE_VALUE,
        "Use relative mode analog mapping instead of absolute analog values");

    cconfig_util_set_int(
        config,
        VIGEM_IIDXIO_CONFIG_TT_ANALOG_RELATIVE_SENSITIVITY_KEY,
        VIGEM_IIDXIO_CONFIG_DEFAULT_TT_ANALOG_RELATIVE_SENSITIVITY_VALUE,
        "Sensitivity value for relative mode (1 to 32767). Tweak if you are "
        "having issues with "
        "jittering/misfiring/unresponsiveness");

    cconfig_util_set_int(
        config,
        VIGEM_IIDXIO_CONFIG_TT_BUTTON_DEBOUNCE_KEY,
        VIGEM_IIDXIO_CONFIG_DEFAULT_TT_BUTTON_DEBOUNCE_VALUE,
        "Button turntable: \"debounce\" value (1 to 50, recommend 20). Tweak "
        "if you are having "
        "issues with TT button misfiring/unresponsiveness");

    cconfig_util_set_int(
        config,
        VIGEM_IIDXIO_CONFIG_TT_BUTTON_THRESHOLD_KEY,
        VIGEM_IIDXIO_CONFIG_DEFAULT_TT_BUTTON_THRESHOLD_VALUE,
        "Button turntable: minimum ticks required within (debounce * 2) ms to "
        "register movement "
        "(1 to 4, recommend 2). Tweak if you button input is too (un-) "
        "responsive");

    cconfig_util_set_bool(
        config,
        VIGEM_IIDXIO_CONFIG_TT_DEBUG_OUTPUT_KEY,
        VIGEM_IIDXIO_CONFIG_DEFAULT_TT_DEBUG_OUTPUT_VALUE,
        "Print verbose debug output to the console for debugging turntable "
        "sensitivity issues");

    cconfig_util_set_bool(
        config,
        VIGEM_IIDXIO_CONFIG_CAB_LIGHT_ENABLE_KEYLIGHT_KEY,
        VIGEM_IIDXIO_CONFIG_DEFAULT_CAB_LIGHT_ENABLE_KEYLIGHT_VALUE,
        "Enable input based key lighting");

    cconfig_util_set_int(
        config,
        VIGEM_IIDXIO_CONFIG_CAB_LIGHT_LIGHT_MODE_KEY,
        VIGEM_IIDXIO_CONFIG_DEFAULT_CAB_LIGHT_LIGHT_MODE_VALUE,
        "Different cabinet light modes: 0 = off, 1 = neons sequence, 2 = neons "
        "flash on TT spin");

    cconfig_util_set_str(
        config,
        VIGEM_IIDXIO_CONFIG_CAB_LIGHT_TEXT_16SEG_KEY,
        VIGEM_IIDXIO_CONFIG_DEFAULT_CAB_LIGHT_TEXT_16SEG_VALUE,
        "Display text on 16seg. If text exceeds 9 char display limit, it will "
        "scroll + cycle");

    cconfig_util_set_int(
        config,
        VIGEM_IIDXIO_CONFIG_CAB_LIGHT_TEXT_SCROLL_CYCLE_TIME_MS_KEY,
        VIGEM_IIDXIO_CONFIG_DEFAULT_CAB_LIGHT_TEXT_SCROLL_CYCLE_TIME_MS_VALUE,
        "Cycle time/scroll speed for text exceeding 16seg display length (9) "
        "to scroll from right");
}

static void _vigem_iidxio_config_get(
    struct vigem_iidxio_config *vigem_config, struct cconfig *config)
{
    if (!cconfig_util_get_bool(
            config,
            VIGEM_IIDXIO_CONFIG_TT_ANALOG_RELATIVE_KEY,
            &vigem_config->tt.analog.relative,
            VIGEM_IIDXIO_CONFIG_DEFAULT_TT_ANALOG_RELATIVE_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            VIGEM_IIDXIO_CONFIG_TT_ANALOG_RELATIVE_KEY,
            VIGEM_IIDXIO_CONFIG_DEFAULT_TT_ANALOG_RELATIVE_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            VIGEM_IIDXIO_CONFIG_TT_ANALOG_RELATIVE_SENSITIVITY_KEY,
            &vigem_config->tt.analog.relative_sensitivity,
            VIGEM_IIDXIO_CONFIG_DEFAULT_TT_ANALOG_RELATIVE_SENSITIVITY_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            VIGEM_IIDXIO_CONFIG_TT_ANALOG_RELATIVE_SENSITIVITY_KEY,
            VIGEM_IIDXIO_CONFIG_DEFAULT_TT_ANALOG_RELATIVE_SENSITIVITY_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            VIGEM_IIDXIO_CONFIG_TT_BUTTON_DEBOUNCE_KEY,
            &vigem_config->tt.button.debounce,
            VIGEM_IIDXIO_CONFIG_DEFAULT_TT_BUTTON_DEBOUNCE_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            VIGEM_IIDXIO_CONFIG_TT_BUTTON_DEBOUNCE_KEY,
            VIGEM_IIDXIO_CONFIG_DEFAULT_TT_BUTTON_DEBOUNCE_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            VIGEM_IIDXIO_CONFIG_TT_BUTTON_THRESHOLD_KEY,
            &vigem_config->tt.button.threshold,
            VIGEM_IIDXIO_CONFIG_DEFAULT_TT_BUTTON_THRESHOLD_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            VIGEM_IIDXIO_CONFIG_TT_BUTTON_THRESHOLD_KEY,
            VIGEM_IIDXIO_CONFIG_DEFAULT_TT_BUTTON_THRESHOLD_VALUE);
    }

    if (!cconfig_util_get_bool(
            config,
            VIGEM_IIDXIO_CONFIG_TT_DEBUG_OUTPUT_KEY,
            &vigem_config->tt.debug_output,
            VIGEM_IIDXIO_CONFIG_DEFAULT_TT_DEBUG_OUTPUT_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            VIGEM_IIDXIO_CONFIG_TT_DEBUG_OUTPUT_KEY,
            VIGEM_IIDXIO_CONFIG_DEFAULT_TT_DEBUG_OUTPUT_VALUE);
    }

    if (!cconfig_util_get_bool(
            config,
            VIGEM_IIDXIO_CONFIG_CAB_LIGHT_ENABLE_KEYLIGHT_KEY,
            &vigem_config->cab_light.enable_keylight,
            VIGEM_IIDXIO_CONFIG_DEFAULT_CAB_LIGHT_ENABLE_KEYLIGHT_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            VIGEM_IIDXIO_CONFIG_CAB_LIGHT_ENABLE_KEYLIGHT_KEY,
            VIGEM_IIDXIO_CONFIG_DEFAULT_CAB_LIGHT_ENABLE_KEYLIGHT_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            VIGEM_IIDXIO_CONFIG_CAB_LIGHT_LIGHT_MODE_KEY,
            &vigem_config->cab_light.light_mode,
            VIGEM_IIDXIO_CONFIG_DEFAULT_CAB_LIGHT_LIGHT_MODE_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            VIGEM_IIDXIO_CONFIG_CAB_LIGHT_LIGHT_MODE_KEY,
            VIGEM_IIDXIO_CONFIG_DEFAULT_CAB_LIGHT_LIGHT_MODE_VALUE);
    }

    if (!cconfig_util_get_str(
            config,
            VIGEM_IIDXIO_CONFIG_CAB_LIGHT_TEXT_16SEG_KEY,
            vigem_config->cab_light.text_16seg,
            sizeof(vigem_config->cab_light.text_16seg),
            VIGEM_IIDXIO_CONFIG_DEFAULT_CAB_LIGHT_TEXT_16SEG_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%s'",
            VIGEM_IIDXIO_CONFIG_CAB_LIGHT_TEXT_16SEG_KEY,
            VIGEM_IIDXIO_CONFIG_DEFAULT_CAB_LIGHT_TEXT_16SEG_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            VIGEM_IIDXIO_CONFIG_CAB_LIGHT_TEXT_SCROLL_CYCLE_TIME_MS_KEY,
            &vigem_config->cab_light.text_scroll_cycle_time_ms,
            VIGEM_IIDXIO_CONFIG_DEFAULT_CAB_LIGHT_TEXT_SCROLL_CYCLE_TIME_MS_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            VIGEM_IIDXIO_CONFIG_CAB_LIGHT_TEXT_SCROLL_CYCLE_TIME_MS_KEY,
            VIGEM_IIDXIO_CONFIG_DEFAULT_CAB_LIGHT_TEXT_SCROLL_CYCLE_TIME_MS_VALUE);
    }
}

bool vigem_iidxio_config_get(struct vigem_iidxio_config *config_out)
{
    struct cconfig *config;

    config = cconfig_init();

    _vigem_iidxio_config_init(config);

    if (!cconfig_main_config_init(
            config,
            "--config",
            "vigem-iidxio.conf",
            "--help",
            "-h",
            "vigem-iidxio",
            CCONFIG_CMD_USAGE_OUT_STDOUT)) {
        cconfig_finit(config);
        return false;
    }

    _vigem_iidxio_config_get(config_out, config);

    cconfig_finit(config);

    return true;
}
