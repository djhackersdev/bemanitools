#include <string.h>

#include "cconfig/cconfig-util.h"

#include "iidxhook-util/config-eamuse.h"

#include "iface-core/log.h"

#include "security/mcode.h"

#define IIDXHOOK_CONFIG_EAMUSE_CARD_TYPE_KEY "eamuse.card_type"
#define IIDXHOOK_CONFIG_EAMUSE_SERVER_KEY "eamuse.server"
#define IIDXHOOK_CONFIG_EAMUSE_PCBID_KEY "eamuse.pcbid"
#define IIDXHOOK_CONFIG_EAMUSE_EAMID_KEY "eamuse.eamid"

#define IIDXHOOK_CONFIG_EAMUSE_DEFAULT_CARD_TYPE_VALUE \
    SECURITY_MCODE_GAME_IIDX_9
#define IIDXHOOK_CONFIG_EAMUSE_DEFAULT_SERVER_VALUE "localhost:80"
#define IIDXHOOK_CONFIG_EAMUSE_DEFAULT_PCBID_VALUE security_id_default
#define IIDXHOOK_CONFIG_EAMUSE_DEFAULT_PCBID_VALUE_LEN \
    sizeof(security_id_default)
#define IIDXHOOK_CONFIG_EAMUSE_DEFAULT_EAMID_VALUE security_id_default
#define IIDXHOOK_CONFIG_EAMUSE_DEFAULT_EAMID_VALUE_LEN \
    sizeof(security_id_default)

const struct net_addr iidxhook_eamuse_default_server = {
    .type = NET_ADDR_TYPE_HOSTNAME,
    .hostname.host = "localhost",
    .hostname.port = 80,
};

void iidxhook_util_config_eamuse_init(struct cconfig *config)
{
    cconfig_util_set_str(
        config,
        IIDXHOOK_CONFIG_EAMUSE_CARD_TYPE_KEY,
        IIDXHOOK_CONFIG_EAMUSE_DEFAULT_CARD_TYPE_VALUE,
        "Magnetic card type, format XXX, 3 digit string (supports: C02, "
        "D01, "
        "E11, ECO)");

    cconfig_util_set_str(
        config,
        IIDXHOOK_CONFIG_EAMUSE_SERVER_KEY,
        IIDXHOOK_CONFIG_EAMUSE_DEFAULT_SERVER_VALUE,
        "URL (e.g. http://my.eamuse.server:80/whatever) or IPV4 "
        "(e.g. 127.0.0.1:80) of the target eamuse server. The port is "
        "optional "
        "but defaults to 80.");

    cconfig_util_set_data(
        config,
        IIDXHOOK_CONFIG_EAMUSE_PCBID_KEY,
        (uint8_t *) &IIDXHOOK_CONFIG_EAMUSE_DEFAULT_PCBID_VALUE,
        IIDXHOOK_CONFIG_EAMUSE_DEFAULT_PCBID_VALUE_LEN,
        "PCBID");

    cconfig_util_set_data(
        config,
        IIDXHOOK_CONFIG_EAMUSE_EAMID_KEY,
        (uint8_t *) &IIDXHOOK_CONFIG_EAMUSE_DEFAULT_EAMID_VALUE,
        IIDXHOOK_CONFIG_EAMUSE_DEFAULT_EAMID_VALUE_LEN,
        "EAMID");
}

void iidxhook_util_config_eamuse_get(
    struct iidxhook_util_config_eamuse *config_eamuse, struct cconfig *config)
{
    char server_url[1024];
    char *tmp;
    char *tmp2;

    memset(config_eamuse, 0, sizeof(struct iidxhook_util_config_eamuse));

    if (!cconfig_util_get_str(
            config,
            IIDXHOOK_CONFIG_EAMUSE_CARD_TYPE_KEY,
            config_eamuse->card_type,
            sizeof(config_eamuse->card_type) - 1,
            IIDXHOOK_CONFIG_EAMUSE_DEFAULT_CARD_TYPE_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%s'",
            IIDXHOOK_CONFIG_EAMUSE_CARD_TYPE_KEY,
            IIDXHOOK_CONFIG_EAMUSE_DEFAULT_CARD_TYPE_VALUE);
    }

    if (!cconfig_util_get_str(
            config,
            IIDXHOOK_CONFIG_EAMUSE_SERVER_KEY,
            server_url,
            sizeof(server_url),
            IIDXHOOK_CONFIG_EAMUSE_DEFAULT_SERVER_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%s'",
            IIDXHOOK_CONFIG_EAMUSE_SERVER_KEY,
            IIDXHOOK_CONFIG_EAMUSE_DEFAULT_SERVER_VALUE);
    }

    if (!net_str_parse(server_url, &config_eamuse->server)) {
        memcpy(
            &config_eamuse->server,
            &iidxhook_eamuse_default_server,
            sizeof(config_eamuse->server));
        tmp = net_addr_to_str(&config_eamuse->server);
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default",
            tmp);
        free(tmp);
    }

    if (!cconfig_util_get_data(
            config,
            IIDXHOOK_CONFIG_EAMUSE_PCBID_KEY,
            (uint8_t *) &config_eamuse->pcbid,
            IIDXHOOK_CONFIG_EAMUSE_DEFAULT_PCBID_VALUE_LEN,
            (uint8_t *) &IIDXHOOK_CONFIG_EAMUSE_DEFAULT_PCBID_VALUE)) {
        tmp = security_id_to_str(
            &IIDXHOOK_CONFIG_EAMUSE_DEFAULT_PCBID_VALUE, false);
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default",
            tmp);
        free(tmp);
    }

    if (!security_id_verify(&config_eamuse->pcbid)) {
        tmp = security_id_to_str(
            &IIDXHOOK_CONFIG_EAMUSE_DEFAULT_PCBID_VALUE, false);
        tmp2 = security_id_to_str(&config_eamuse->pcbid, false);
        log_warning(
            "PCBID verification of '%s' failed, fallback to default "
            "PCBID '%s'",
            tmp2,
            tmp);
        free(tmp);
        free(tmp2);
    }

    if (!cconfig_util_get_data(
            config,
            IIDXHOOK_CONFIG_EAMUSE_EAMID_KEY,
            (uint8_t *) &config_eamuse->eamid,
            IIDXHOOK_CONFIG_EAMUSE_DEFAULT_EAMID_VALUE_LEN,
            (uint8_t *) &IIDXHOOK_CONFIG_EAMUSE_DEFAULT_EAMID_VALUE)) {
        tmp = security_id_to_str(
            &IIDXHOOK_CONFIG_EAMUSE_DEFAULT_EAMID_VALUE, false);
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default",
            tmp);
        free(tmp);
    }

    if (!security_id_verify(&config_eamuse->eamid)) {
        tmp = security_id_to_str(
            &IIDXHOOK_CONFIG_EAMUSE_DEFAULT_EAMID_VALUE, false);
        tmp2 = security_id_to_str(&config_eamuse->eamid, false);
        log_warning(
            "EAMID verification of '%s' failed, fallback to default "
            "EAMID '%s'",
            tmp2,
            tmp);
        free(tmp);
        free(tmp2);
    }
}
