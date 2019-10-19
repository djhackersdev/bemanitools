#include "cconfig/cconfig-util.h"

#include "iidxhook2/config-iidxhook2.h"

#include "util/log.h"

#define IIDXHOOK_CONFIG_MISC_DISTORTED_MS_BG_FIX_KEY "misc.distorted_ms_bg_fix"

#define IIDXHOOK_CONFIG_MISC_DEFAULT_DISTORTED_MS_BG_FIX_VALUE false

void iidxhook_config_iidxhook2_init(struct cconfig *config)
{
    cconfig_util_set_bool(
        config,
        IIDXHOOK_CONFIG_MISC_DISTORTED_MS_BG_FIX_KEY,
        IIDXHOOK_CONFIG_MISC_DEFAULT_DISTORTED_MS_BG_FIX_VALUE,
        "Fix broken 3D background on DistorteD's music select (if "
        "appearing "
        "completely black)");
}

void iidxhook_config_iidxhook2_get(
    struct iidxhook_config_iidxhook2 *config_iidxhook2, struct cconfig *config)
{
    if (!cconfig_util_get_bool(
            config,
            IIDXHOOK_CONFIG_MISC_DISTORTED_MS_BG_FIX_KEY,
            &config_iidxhook2->distorted_ms_bg_fix,
            IIDXHOOK_CONFIG_MISC_DEFAULT_DISTORTED_MS_BG_FIX_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK_CONFIG_MISC_DISTORTED_MS_BG_FIX_KEY,
            IIDXHOOK_CONFIG_MISC_DEFAULT_DISTORTED_MS_BG_FIX_VALUE);
    }
}