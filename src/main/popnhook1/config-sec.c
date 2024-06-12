#include <stdio.h>
#include <string.h>

#include "cconfig/cconfig-util.h"

#include "iface-core/log.h"

#include "popnhook1/config-sec.h"

#include "security/mcode.h"
#include "security/rp.h"

#include "util/mem.h"

#define POPNHOOK1_CONFIG_SEC_BLACK_PLUG_MCODE_KEY "sec.black_plug_mcode"

#define POPNHOOK1_CONFIG_SEC_DEFAULT_BLACK_PLUG_MCODE_VALUE \
    popnhook1_security_default_black_plug_mcode

static const struct security_mcode popnhook1_security_default_black_plug_mcode =
    {
        .header = SECURITY_MCODE_HEADER,
        .unkn = SECURITY_MCODE_UNKN_Q,
        .game = SECURITY_MCODE_GAME_POPN_15,
        .region = SECURITY_MCODE_REGION_JAPAN,
        .cabinet = SECURITY_MCODE_CABINET_A,
        .revision = SECURITY_MCODE_REVISION_A,
};

void popnhook1_config_sec_init(struct cconfig *config)
{
    char *tmp;

    tmp = security_mcode_to_str(
        &POPNHOOK1_CONFIG_SEC_DEFAULT_BLACK_PLUG_MCODE_VALUE);

    cconfig_util_set_str(
        config,
        POPNHOOK1_CONFIG_SEC_BLACK_PLUG_MCODE_KEY,
        tmp,
        "Security black plug mcode id string (e.g. GQC02JAA).");

    free(tmp);
}

void popnhook1_config_sec_get(
    struct popnhook1_config_sec *config_sec, struct cconfig *config)
{
    char tmp_mcode[sizeof(struct security_mcode) + 1];
    char *tmp_str;

    tmp_str = security_mcode_to_str(
        &POPNHOOK1_CONFIG_SEC_DEFAULT_BLACK_PLUG_MCODE_VALUE);

    if (!cconfig_util_get_str(
            config,
            POPNHOOK1_CONFIG_SEC_BLACK_PLUG_MCODE_KEY,
            tmp_mcode,
            sizeof(tmp_mcode) - 1,
            tmp_str)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%s'",
            POPNHOOK1_CONFIG_SEC_BLACK_PLUG_MCODE_KEY,
            tmp_str);
    }

    if (!security_mcode_parse(tmp_mcode, &config_sec->black_plug_mcode)) {
        log_warning(
            "Invalid format for value of key '%s' specified, fallback "
            "to default '%s'",
            POPNHOOK1_CONFIG_SEC_BLACK_PLUG_MCODE_KEY,
            tmp_str);
        memcpy(
            &config_sec->black_plug_mcode,
            &POPNHOOK1_CONFIG_SEC_DEFAULT_BLACK_PLUG_MCODE_VALUE,
            sizeof(POPNHOOK1_CONFIG_SEC_DEFAULT_BLACK_PLUG_MCODE_VALUE));
    }

    free(tmp_str);
}
