#include <stdio.h>
#include <string.h>

#include "cconfig/cconfig-util.h"

#include "iidxhook-util/config-sec.h"

#include "security/mcode.h"
#include "security/rp.h"

#include "util/log.h"
#include "util/mem.h"

#define IIDXHOOK_CONFIG_SEC_BOOT_VERSION_KEY "sec.boot_version"
#define IIDXHOOK_CONFIG_SEC_BOOT_SEEDS_KEY "sec.boot_seeds"
#define IIDXHOOK_CONFIG_SEC_BLACK_PLUG_MCODE_KEY "sec.black_plug_mcode"

/* Use C02 defaults */
#define IIDXHOOK_CONFIG_SEC_DEFAULT_BOOT_VERSION_VALUE iidxhook_security_default_boot_version
#define IIDXHOOK_CONFIG_SEC_DEFAULT_BOOT_SEEDS_VALUE iidxhook_security_default_boot_seeds
#define IIDXHOOK_CONFIG_SEC_DEFAULT_BLACK_PLUG_MCODE_VALUE iidxhook_security_default_black_plug_mcode

/* IIDX 09 default */
static const struct security_mcode iidxhook_security_default_boot_version = {
    .header = SECURITY_MCODE_HEADER,
    .unkn = SECURITY_MCODE_UNKN_E,
    .game = SECURITY_MCODE_GAME_IIDX_9,
    .region = SECURITY_MCODE_FIELD_BLANK,
    .cabinet = SECURITY_MCODE_FIELD_BLANK,
    .revision = SECURITY_MCODE_FIELD_BLANK,
};

/* IIDX 09 default */
static const uint32_t iidxhook_security_default_boot_seeds[3] = {0, 0, 0};

/* IIDX 09 default */
static const struct security_mcode iidxhook_security_default_black_plug_mcode = {
    .header = SECURITY_MCODE_HEADER,
    .unkn = SECURITY_MCODE_UNKN_Q,
    .game = SECURITY_MCODE_GAME_IIDX_9,
    .region = SECURITY_MCODE_REGION_JAPAN,
    .cabinet = SECURITY_MCODE_CABINET_A,
    .revision = SECURITY_MCODE_REVISION_A,
};

static char* iidxhook_config_boot_seeds_to_str(const uint32_t* seeds)
{
    char* res;
    size_t len;

    len = snprintf(NULL, 0, "%d:%d:%d", seeds[0], seeds[1], seeds[2]);
    res = xmalloc(len + 1);
    sprintf(res, "%d:%d:%d", seeds[0], seeds[1], seeds[2]);

    return res;
}

static bool iidxhook_config_boot_seeds_parse(const char* str, uint32_t* seeds)
{
    if (strlen(str) < 5) {
        return false;
    }

    if (str[1] != ':' || str[3] != ':') {
        return false;
    } else {
        seeds[0] = str[0] - '0';
        seeds[1] = str[2] - '0';
        seeds[2] = str[4] - '0';
        return true;
    }
}

void iidxhook_config_sec_init(struct cconfig* config)
{
    char* tmp;

    tmp = security_mcode_to_str(
        &IIDXHOOK_CONFIG_SEC_DEFAULT_BOOT_VERSION_VALUE);

    cconfig_util_set_str(config, 
        IIDXHOOK_CONFIG_SEC_BOOT_VERSION_KEY, tmp,
        "Security boot version (e.g. GEC02).");

    free(tmp);
    
    tmp = iidxhook_config_boot_seeds_to_str(
        IIDXHOOK_CONFIG_SEC_DEFAULT_BOOT_SEEDS_VALUE);

    cconfig_util_set_str(config, 
        IIDXHOOK_CONFIG_SEC_BOOT_SEEDS_KEY, tmp,
        "Security boot seeds for ezusb, format: X:X:X where X is a number of "
        "0-9 (e.g. 0:0:0).");

    free(tmp);

    tmp = security_mcode_to_str(
        &IIDXHOOK_CONFIG_SEC_DEFAULT_BLACK_PLUG_MCODE_VALUE);

    cconfig_util_set_str(config, 
        IIDXHOOK_CONFIG_SEC_BLACK_PLUG_MCODE_KEY, tmp,
        "Security black plug mcode id string (e.g. GQC02JAA).");

    free(tmp);
}

void iidxhook_config_sec_get(struct iidxhook_config_sec* config_sec, 
        struct cconfig* config)
{
    char tmp_seeds[6];
    char tmp_mcode[sizeof(struct security_mcode) + 1];
    char* tmp_str;

    tmp_str = security_mcode_to_str(
        &IIDXHOOK_CONFIG_SEC_DEFAULT_BOOT_VERSION_VALUE);

    if (!cconfig_util_get_str(config, IIDXHOOK_CONFIG_SEC_BOOT_VERSION_KEY, 
            tmp_mcode, sizeof(tmp_mcode) - 1, tmp_str)) {
        log_warning("Invalid value for key '%s' specified, fallback "
            "to default '%s'", IIDXHOOK_CONFIG_SEC_BOOT_VERSION_KEY, tmp_str);
    }

    if (!security_mcode_parse(tmp_mcode, &config_sec->boot_version)) {
        log_warning("Invalid format for value of key '%s' specified, fallback "
            "to default '%s'", IIDXHOOK_CONFIG_SEC_BOOT_VERSION_KEY, tmp_str);
        memcpy(&config_sec->boot_version, 
            &IIDXHOOK_CONFIG_SEC_DEFAULT_BOOT_VERSION_VALUE,
            sizeof(IIDXHOOK_CONFIG_SEC_DEFAULT_BOOT_VERSION_VALUE));
    }

    free(tmp_str);

    tmp_str = iidxhook_config_boot_seeds_to_str(
        IIDXHOOK_CONFIG_SEC_DEFAULT_BOOT_SEEDS_VALUE);

    if (!cconfig_util_get_str(config, IIDXHOOK_CONFIG_SEC_BOOT_SEEDS_KEY, 
            tmp_seeds, sizeof(tmp_seeds) - 1, tmp_str)) {
        log_warning("Invalid value for key '%s' specified, fallback "
            "to default '%s'", IIDXHOOK_CONFIG_SEC_BOOT_SEEDS_KEY, tmp_str);
    }

    if (!iidxhook_config_boot_seeds_parse(tmp_seeds, config_sec->boot_seeds)) {
        log_warning("Invalid format for value of key '%s' specified, fallback "
            "to default '%s'", IIDXHOOK_CONFIG_SEC_BOOT_SEEDS_KEY, tmp_str);
        memcpy(config_sec->boot_seeds, 
            IIDXHOOK_CONFIG_SEC_DEFAULT_BOOT_SEEDS_VALUE,
            sizeof(IIDXHOOK_CONFIG_SEC_DEFAULT_BOOT_SEEDS_VALUE));
    }

    free(tmp_str);

    tmp_str = security_mcode_to_str(
        &IIDXHOOK_CONFIG_SEC_DEFAULT_BLACK_PLUG_MCODE_VALUE);

    if (!cconfig_util_get_str(config, IIDXHOOK_CONFIG_SEC_BLACK_PLUG_MCODE_KEY, 
            tmp_mcode, sizeof(tmp_mcode) - 1, tmp_str)) {
        log_warning("Invalid value for key '%s' specified, fallback "
            "to default '%s'", IIDXHOOK_CONFIG_SEC_BLACK_PLUG_MCODE_KEY,
            tmp_str);
    }

    if (!security_mcode_parse(tmp_mcode, &config_sec->black_plug_mcode)) {
        log_warning("Invalid format for value of key '%s' specified, fallback "
            "to default '%s'", IIDXHOOK_CONFIG_SEC_BLACK_PLUG_MCODE_KEY,
            tmp_str);
        memcpy(&config_sec->black_plug_mcode, 
            &IIDXHOOK_CONFIG_SEC_DEFAULT_BLACK_PLUG_MCODE_VALUE,
            sizeof(IIDXHOOK_CONFIG_SEC_DEFAULT_BLACK_PLUG_MCODE_VALUE));
    }

    free(tmp_str);
}
