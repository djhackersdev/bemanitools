#include "cconfig/cconfig-util.h"

#include "iidxhook-util/config-misc.h"

#include "util/log.h"

#define IIDXHOOK_CONFIG_MISC_DISABLE_CLOCK_SET_KEY "misc.disable_clock_set"
#define IIDXHOOK_CONFIG_MISC_RTEFFECT_STUB_KEY "misc.rteffect_stub"
#define IIDXHOOK_CONFIG_MISC_SETTINGS_PATH_STUB_KEY "misc.settings_path"

#define IIDXHOOK_CONFIG_MISC_DEFAULT_DISABLE_CLOCK_SET_VALUE false
#define IIDXHOOK_CONFIG_MISC_DEFAULT_RTEFFECT_STUB_VALUE false
#define IIDXHOOK_CONFIG_MISC_DEFAULT_SETTINGS_PATH_STUB_VALUE ".\\"

void iidxhook_config_misc_init(struct cconfig *config)
{
    cconfig_util_set_bool(
        config,
        IIDXHOOK_CONFIG_MISC_DISABLE_CLOCK_SET_KEY,
        IIDXHOOK_CONFIG_MISC_DEFAULT_DISABLE_CLOCK_SET_VALUE,
        "Disable operator clock setting system clock time");

    cconfig_util_set_bool(
        config,
        IIDXHOOK_CONFIG_MISC_RTEFFECT_STUB_KEY,
        IIDXHOOK_CONFIG_MISC_DEFAULT_RTEFFECT_STUB_VALUE,
        "Stub calls to rteffect.dll (10th to DistorteD)");

    cconfig_util_set_str(
        config,
        IIDXHOOK_CONFIG_MISC_SETTINGS_PATH_STUB_KEY,
        IIDXHOOK_CONFIG_MISC_DEFAULT_SETTINGS_PATH_STUB_VALUE,
        "Path to store the settings, e.g. bookkeeping, operator settings. d:, "
        "e: and f: drive configuration/settings data");
}

void iidxhook_config_misc_get(
    struct iidxhook_config_misc *config_misc, struct cconfig *config)
{
    if (!cconfig_util_get_bool(
            config,
            IIDXHOOK_CONFIG_MISC_DISABLE_CLOCK_SET_KEY,
            &config_misc->disable_clock_set,
            IIDXHOOK_CONFIG_MISC_DEFAULT_DISABLE_CLOCK_SET_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK_CONFIG_MISC_DISABLE_CLOCK_SET_KEY,
            IIDXHOOK_CONFIG_MISC_DEFAULT_DISABLE_CLOCK_SET_VALUE);
    }

    if (!cconfig_util_get_bool(
            config,
            IIDXHOOK_CONFIG_MISC_RTEFFECT_STUB_KEY,
            &config_misc->rteffect_stub,
            IIDXHOOK_CONFIG_MISC_DEFAULT_RTEFFECT_STUB_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK_CONFIG_MISC_RTEFFECT_STUB_KEY,
            IIDXHOOK_CONFIG_MISC_DEFAULT_RTEFFECT_STUB_VALUE);
    }

    if (!cconfig_util_get_str(
            config,
            IIDXHOOK_CONFIG_MISC_SETTINGS_PATH_STUB_KEY,
            config_misc->settings_path,
            sizeof(config_misc->settings_path),
            IIDXHOOK_CONFIG_MISC_DEFAULT_SETTINGS_PATH_STUB_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%s'",
            IIDXHOOK_CONFIG_MISC_SETTINGS_PATH_STUB_KEY,
            IIDXHOOK_CONFIG_MISC_DEFAULT_SETTINGS_PATH_STUB_VALUE);
    }
}
