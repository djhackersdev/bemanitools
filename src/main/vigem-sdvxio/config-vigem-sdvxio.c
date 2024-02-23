#include "cconfig/cconfig-main.h"
#include "cconfig/cconfig-util.h"

#include "core/log.h"

#include "vigem-sdvxio/config-vigem-sdvxio.h"

#define VIGEM_SDVXIO_CONFIG_ENABLE_KEYLIGHT_KEY "sdvxio.enable_keylight"
#define VIGEM_SDVXIO_CONFIG_RELATIVE_ANALOG_KEY "sdvxio.use_relative_analog"
#define VIGEM_SDVXIO_CONFIG_PWM_WINGS_KEY "sdvxio.pwm_wings"
#define VIGEM_SDVXIO_CONFIG_PWM_CONTROLLER_KEY "sdvxio.pwm_controller"
#define VIGEM_SDVXIO_CONFIG_AMP_VOLUME_KEY "sdvxio.amp_volume"

#define VIGEM_SDVXIO_CONFIG_DEFAULT_ENABLE_KEYLIGHT_VALUE true
#define VIGEM_SDVXIO_CONFIG_DEFAULT_RELATIVE_ANALOG_VALUE false
#define VIGEM_SDVXIO_CONFIG_DEFAULT_PWM_WINGS_VALUE 128
#define VIGEM_SDVXIO_CONFIG_DEFAULT_PWM_CONTROLLER_VALUE 64
#define VIGEM_SDVXIO_CONFIG_DEFAULT_AMP_VOLUME_VALUE 48

static void vigem_sdvxio_config_init(struct cconfig *config)
{
    cconfig_util_set_bool(
        config,
        VIGEM_SDVXIO_CONFIG_ENABLE_KEYLIGHT_KEY,
        VIGEM_SDVXIO_CONFIG_DEFAULT_ENABLE_KEYLIGHT_VALUE,
        "Enable input based key lighting");

    cconfig_util_set_bool(
        config,
        VIGEM_SDVXIO_CONFIG_RELATIVE_ANALOG_KEY,
        VIGEM_SDVXIO_CONFIG_DEFAULT_RELATIVE_ANALOG_VALUE,
        "Use relative mode analog mapping");

    cconfig_util_set_int(
        config,
        VIGEM_SDVXIO_CONFIG_PWM_WINGS_KEY,
        VIGEM_SDVXIO_CONFIG_DEFAULT_PWM_WINGS_VALUE,
        "Brightness to set wings to (0-255)");

    cconfig_util_set_int(
        config,
        VIGEM_SDVXIO_CONFIG_PWM_CONTROLLER_KEY,
        VIGEM_SDVXIO_CONFIG_DEFAULT_PWM_CONTROLLER_VALUE,
        "Brightness to set control deck to (0-255)");

    cconfig_util_set_int(
        config,
        VIGEM_SDVXIO_CONFIG_AMP_VOLUME_KEY,
        VIGEM_SDVXIO_CONFIG_DEFAULT_AMP_VOLUME_VALUE,
        "SDVXIO digital amp volume (0-96) 0 is high, 96 is low.");
}

static void vigem_sdvxio_config_get(
    struct vigem_sdvxio_config *vigem_config, struct cconfig *config)
{
    if (!cconfig_util_get_bool(
            config,
            VIGEM_SDVXIO_CONFIG_ENABLE_KEYLIGHT_KEY,
            &vigem_config->enable_keylight,
            VIGEM_SDVXIO_CONFIG_DEFAULT_ENABLE_KEYLIGHT_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            VIGEM_SDVXIO_CONFIG_ENABLE_KEYLIGHT_KEY,
            VIGEM_SDVXIO_CONFIG_DEFAULT_ENABLE_KEYLIGHT_VALUE);
    }

    if (!cconfig_util_get_bool(
            config,
            VIGEM_SDVXIO_CONFIG_RELATIVE_ANALOG_KEY,
            &vigem_config->relative_analog,
            VIGEM_SDVXIO_CONFIG_DEFAULT_RELATIVE_ANALOG_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            VIGEM_SDVXIO_CONFIG_RELATIVE_ANALOG_KEY,
            VIGEM_SDVXIO_CONFIG_DEFAULT_RELATIVE_ANALOG_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            VIGEM_SDVXIO_CONFIG_PWM_WINGS_KEY,
            &vigem_config->pwm_wings,
            VIGEM_SDVXIO_CONFIG_DEFAULT_PWM_WINGS_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            VIGEM_SDVXIO_CONFIG_PWM_WINGS_KEY,
            VIGEM_SDVXIO_CONFIG_DEFAULT_PWM_WINGS_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            VIGEM_SDVXIO_CONFIG_PWM_CONTROLLER_KEY,
            &vigem_config->pwm_controller,
            VIGEM_SDVXIO_CONFIG_DEFAULT_PWM_CONTROLLER_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            VIGEM_SDVXIO_CONFIG_PWM_CONTROLLER_KEY,
            VIGEM_SDVXIO_CONFIG_DEFAULT_PWM_CONTROLLER_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            VIGEM_SDVXIO_CONFIG_AMP_VOLUME_KEY,
            &vigem_config->amp_volume,
            VIGEM_SDVXIO_CONFIG_DEFAULT_AMP_VOLUME_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            VIGEM_SDVXIO_CONFIG_AMP_VOLUME_KEY,
            VIGEM_SDVXIO_CONFIG_DEFAULT_AMP_VOLUME_VALUE);
    }
}

bool get_vigem_sdvxio_config(struct vigem_sdvxio_config *config_out)
{
    struct cconfig *config;

    config = cconfig_init();

    vigem_sdvxio_config_init(config);

    if (!cconfig_main_config_init(
            config,
            "--config",
            "vigem-sdvxio.conf",
            "--help",
            "-h",
            "vigem-sdvxio",
            CCONFIG_CMD_USAGE_OUT_STDOUT)) {
        cconfig_finit(config);
        return false;
    }

    vigem_sdvxio_config_get(config_out, config);

    cconfig_finit(config);

    if (config_out->pwm_controller > 255) {
        config_out->pwm_controller = 255;
    }

    if (config_out->pwm_wings > 255) {
        config_out->pwm_wings = 255;
    }
    return true;
}
