#include <string.h>

#include "cconfig/cconfig-util.h"

#include "jbhook1/config-security.h"

#include "jbhook-util/security.h"

#include "security/mcode.h"

#include "util/log.h"
#include "util/net.h"

#define JBHOOK1_CONFIG_SECURITY_MCODE_KEY "security.mcode"

#define JBHOOK1_CONFIG_SECURITY_DEFAULT_MCODE_VALUE \
    jbhook_util_security_default_mcode

void jbhook1_config_security_init(struct cconfig *config)
{
    char *tmp;

    tmp = security_mcode_to_str(&JBHOOK1_CONFIG_SECURITY_DEFAULT_MCODE_VALUE);

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

    tmp_default =
        security_mcode_to_str(&JBHOOK1_CONFIG_SECURITY_DEFAULT_MCODE_VALUE);

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
            &JBHOOK1_CONFIG_SECURITY_DEFAULT_MCODE_VALUE,
            sizeof(struct security_mcode));
    }

    free(tmp_default);
}
