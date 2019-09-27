#include "cconfig/cconfig-util.h"

#include "iidxhook1/config-iidxhook1.h"

#include "util/log.h"

#define IIDXHOOK_CONFIG_MISC_HAPPY_SKY_MS_BG_FIX_KEY "misc.happy_sky_ms_bg_fix"
#define IIDXHOOK_CONFIG_MISC_USE_D3D9_HOOKS_KEY "misc.use_d3d9_hooks"

#define IIDXHOOK_CONFIG_MISC_DEFAULT_HAPPY_SKY_MS_BG_FIX_VALUE false
#define IIDXHOOK_CONFIG_MISC_DEFAULT_USE_D3D9_HOOKS_VALUE false

void iidxhook_config_iidxhook1_init(struct cconfig* config)
{
    cconfig_util_set_bool(config, 
        IIDXHOOK_CONFIG_MISC_HAPPY_SKY_MS_BG_FIX_KEY,
        IIDXHOOK_CONFIG_MISC_DEFAULT_HAPPY_SKY_MS_BG_FIX_VALUE,
        "Fix broken 3D background on Happy Sky's music select (if appearing "
        "completely white)");

    cconfig_util_set_bool(config, 
        IIDXHOOK_CONFIG_MISC_USE_D3D9_HOOKS_KEY,
        IIDXHOOK_CONFIG_MISC_DEFAULT_USE_D3D9_HOOKS_VALUE,
        "Use d3d9 hooks instead of d3d8 to enable d3d9 hook features not available on d3d8 (e.g. upscaling). Requires "
        " d3d8to9 wrapper library to be used with this game.");
}

void iidxhook_config_iidxhook1_get(
        struct iidxhook_config_iidxhook1* config_iidxhook1, 
        struct cconfig* config)
{
    if (!cconfig_util_get_bool(config,
            IIDXHOOK_CONFIG_MISC_HAPPY_SKY_MS_BG_FIX_KEY, 
            &config_iidxhook1->happy_sky_ms_bg_fix, 
            IIDXHOOK_CONFIG_MISC_DEFAULT_HAPPY_SKY_MS_BG_FIX_VALUE)) {
        log_warning("Invalid value for key '%s' specified, fallback "
            "to default '%d'", IIDXHOOK_CONFIG_MISC_HAPPY_SKY_MS_BG_FIX_KEY,
            IIDXHOOK_CONFIG_MISC_DEFAULT_HAPPY_SKY_MS_BG_FIX_VALUE);
    }

    if (!cconfig_util_get_bool(config,
            IIDXHOOK_CONFIG_MISC_USE_D3D9_HOOKS_KEY, 
            &config_iidxhook1->use_d3d9_hooks, 
            IIDXHOOK_CONFIG_MISC_DEFAULT_USE_D3D9_HOOKS_VALUE)) {
        log_warning("Invalid value for key '%s' specified, fallback "
            "to default '%d'", IIDXHOOK_CONFIG_MISC_USE_D3D9_HOOKS_KEY,
            IIDXHOOK_CONFIG_MISC_DEFAULT_USE_D3D9_HOOKS_VALUE);
    }
}