#include <string.h>

#include "cconfig/cconfig-util.h"

#include "iface-core/log.h"

#include "jbhook1/config-security.h"

#include "security/mcode.h"

#include "util/net.h"

#define JBHOOK1_CONFIG_SECURITY_MCODE_KEY "security.mcode"

static const struct security_mcode security_mcode_h44 = {
    .header = SECURITY_MCODE_HEADER,
    .unkn = SECURITY_MCODE_UNKN_C,
    .game = SECURITY_MCODE_GAME_JB_1,
    .region = SECURITY_MCODE_REGION_JAPAN,
    .cabinet = SECURITY_MCODE_CABINET_A,
    .revision = SECURITY_MCODE_REVISION_B,
};

void jbhook1_config_security_init(struct cconfig *config)
{
    char *tmp;

    tmp = security_mcode_to_str(&security_mcode_h44);

    cconfig_util_set_str(
        config,
        JBHOOK1_CONFIG_SECURITY_MCODE_KEY,
        tmp,
        "Mcode of the game to run.");

    free(tmp);
}

void jbhook1_config_security_get(
    struct jbhook1_config_security *config_security, struct cconfig *config)
{
    char *tmp_default;
    char mcode[9];

    tmp_default = security_mcode_to_str(&security_mcode_h44);

    if (!cconfig_util_get_str(
            config,
            JBHOOK1_CONFIG_SECURITY_MCODE_KEY,
            mcode,
            sizeof(mcode) - 1,
            tmp_default)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%s'",
            JBHOOK1_CONFIG_SECURITY_MCODE_KEY,
            tmp_default);
    }

    mcode[8] = '\0';

    if (!security_mcode_parse(mcode, &config_security->mcode)) {
        log_warning(
            "Invalid mcode '%s' specified, fallback to default '%s'",
            mcode,
            tmp_default);

        memcpy(
            &config_security->mcode,
            &security_mcode_h44,
            sizeof(struct security_mcode));
    }

    free(tmp_default);
}
