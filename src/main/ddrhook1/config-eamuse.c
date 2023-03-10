#include <string.h>

#include "cconfig/cconfig-util.h"

#include "ddrhook1/config-eamuse.h"

#include "util/log.h"
#include "util/net.h"

#define DDRHOOK1_CONFIG_EAMUSE_SERVER_KEY "eamuse.server"
#define DDRHOOK1_CONFIG_EAMUSE_PCBID_KEY "eamuse.pcbid"
#define DDRHOOK1_CONFIG_EAMUSE_EAMID_KEY "eamuse.eamid"

#define DDRHOOK1_CONFIG_EAMUSE_DEFAULT_SERVER_VALUE "localhost:80"
#define DDRHOOK1_CONFIG_EAMUSE_DEFAULT_PCBID_VALUE security_id_default
#define DDRHOOK1_CONFIG_EAMUSE_DEFAULT_PCBID_VALUE_LEN \
    sizeof(security_id_default)
#define DDRHOOK1_CONFIG_EAMUSE_DEFAULT_EAMID_VALUE security_id_default
#define DDRHOOK1_CONFIG_EAMUSE_DEFAULT_EAMID_VALUE_LEN \
    sizeof(security_id_default)

const struct net_addr ddrhook1_eamuse_default_server = {
    .type = NET_ADDR_TYPE_HOSTNAME,
    .hostname.host = "localhost",
    .hostname.port = 80,
};

void ddrhook1_config_eamuse_init(struct cconfig *config)
{
    cconfig_util_set_str(
        config,
        DDRHOOK1_CONFIG_EAMUSE_SERVER_KEY,
        DDRHOOK1_CONFIG_EAMUSE_DEFAULT_SERVER_VALUE,
        "URL (e.g. http://my.eamuse.server:80/whatever) or IPV4 "
        "(e.g. 127.0.0.1:80) of the target eamuse server. The port is "
        "optional "
        "but defaults to 80.");

    cconfig_util_set_data(
        config,
        DDRHOOK1_CONFIG_EAMUSE_PCBID_KEY,
        (uint8_t *) &DDRHOOK1_CONFIG_EAMUSE_DEFAULT_PCBID_VALUE,
        DDRHOOK1_CONFIG_EAMUSE_DEFAULT_PCBID_VALUE_LEN,
        "PCBID");

    cconfig_util_set_data(
        config,
        DDRHOOK1_CONFIG_EAMUSE_EAMID_KEY,
        (uint8_t *) &DDRHOOK1_CONFIG_EAMUSE_DEFAULT_EAMID_VALUE,
        DDRHOOK1_CONFIG_EAMUSE_DEFAULT_EAMID_VALUE_LEN,
        "EAMID");
}

void ddrhook1_config_eamuse_get(
    struct ddrhook1_config_eamuse *config_eamuse, struct cconfig *config)
{
    char server_url[1024];
    char *tmp;
    char *tmp2;

    memset(config_eamuse, 0, sizeof(struct ddrhook1_config_eamuse));

    if (!cconfig_util_get_str(
            config,
            DDRHOOK1_CONFIG_EAMUSE_SERVER_KEY,
            server_url,
            sizeof(server_url),
            DDRHOOK1_CONFIG_EAMUSE_DEFAULT_SERVER_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%s'",
            DDRHOOK1_CONFIG_EAMUSE_SERVER_KEY,
            DDRHOOK1_CONFIG_EAMUSE_DEFAULT_SERVER_VALUE);
    }

    if (!net_str_parse(server_url, &config_eamuse->server)) {
        memcpy(
            &config_eamuse->server,
            &ddrhook1_eamuse_default_server,
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
            DDRHOOK1_CONFIG_EAMUSE_PCBID_KEY,
            (uint8_t *) &config_eamuse->pcbid,
            DDRHOOK1_CONFIG_EAMUSE_DEFAULT_PCBID_VALUE_LEN,
            (uint8_t *) &DDRHOOK1_CONFIG_EAMUSE_DEFAULT_PCBID_VALUE)) {
        tmp = security_id_to_str(
            &DDRHOOK1_CONFIG_EAMUSE_DEFAULT_PCBID_VALUE, false);
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default",
            tmp);
        free(tmp);
    }

    if (!security_id_verify(&config_eamuse->pcbid)) {
        tmp = security_id_to_str(
            &DDRHOOK1_CONFIG_EAMUSE_DEFAULT_PCBID_VALUE, false);
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
            DDRHOOK1_CONFIG_EAMUSE_EAMID_KEY,
            (uint8_t *) &config_eamuse->eamid,
            DDRHOOK1_CONFIG_EAMUSE_DEFAULT_EAMID_VALUE_LEN,
            (uint8_t *) &DDRHOOK1_CONFIG_EAMUSE_DEFAULT_EAMID_VALUE)) {
        tmp = security_id_to_str(
            &DDRHOOK1_CONFIG_EAMUSE_DEFAULT_EAMID_VALUE, false);
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default",
            tmp);
        free(tmp);
    }

    if (!security_id_verify(&config_eamuse->eamid)) {
        tmp = security_id_to_str(
            &DDRHOOK1_CONFIG_EAMUSE_DEFAULT_EAMID_VALUE, false);
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
