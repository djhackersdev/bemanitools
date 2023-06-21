#include <string.h>

#include "cconfig/cconfig-util.h"

#include "jbhook1/config-eamuse.h"

#include "util/log.h"
#include "util/net.h"

#define JBHOOK1_CONFIG_EAMUSE_SERVER_KEY "eamuse.server"
#define JBHOOK1_CONFIG_EAMUSE_PCBID_KEY "eamuse.pcbid"
#define JBHOOK1_CONFIG_EAMUSE_EAMID_KEY "eamuse.eamid"

#define JBHOOK1_CONFIG_EAMUSE_DEFAULT_SERVER_VALUE "localhost:80"
#define JBHOOK1_CONFIG_EAMUSE_DEFAULT_PCBID_VALUE security_id_default
#define JBHOOK1_CONFIG_EAMUSE_DEFAULT_PCBID_VALUE_LEN \
    sizeof(security_id_default)
#define JBHOOK1_CONFIG_EAMUSE_DEFAULT_EAMID_VALUE security_id_default
#define JBHOOK1_CONFIG_EAMUSE_DEFAULT_EAMID_VALUE_LEN \
    sizeof(security_id_default)

const struct net_addr jbhook1_eamuse_default_server = {
    .type = NET_ADDR_TYPE_HOSTNAME,
    .hostname.host = "localhost",
    .hostname.port = 80,
};

void jbhook1_config_eamuse_init(struct cconfig *config)
{
    cconfig_util_set_str(
        config,
        JBHOOK1_CONFIG_EAMUSE_SERVER_KEY,
        JBHOOK1_CONFIG_EAMUSE_DEFAULT_SERVER_VALUE,
        "URL (e.g. http://my.eamuse.server:80/whatever) or IPV4 "
        "(e.g. 127.0.0.1:80) of the target eamuse server. The port is "
        "optional "
        "but defaults to 80.");

    cconfig_util_set_data(
        config,
        JBHOOK1_CONFIG_EAMUSE_PCBID_KEY,
        (uint8_t *) &JBHOOK1_CONFIG_EAMUSE_DEFAULT_PCBID_VALUE,
        JBHOOK1_CONFIG_EAMUSE_DEFAULT_PCBID_VALUE_LEN,
        "PCBID");

    cconfig_util_set_data(
        config,
        JBHOOK1_CONFIG_EAMUSE_EAMID_KEY,
        (uint8_t *) &JBHOOK1_CONFIG_EAMUSE_DEFAULT_EAMID_VALUE,
        JBHOOK1_CONFIG_EAMUSE_DEFAULT_EAMID_VALUE_LEN,
        "EAMID");
}

void jbhook1_config_eamuse_get(
    struct jbhook1_config_eamuse *config_eamuse, struct cconfig *config)
{
    char server_url[1024];
    char *tmp;
    char *tmp2;

    memset(config_eamuse, 0, sizeof(struct jbhook1_config_eamuse));

    if (!cconfig_util_get_str(
            config,
            JBHOOK1_CONFIG_EAMUSE_SERVER_KEY,
            server_url,
            sizeof(server_url),
            JBHOOK1_CONFIG_EAMUSE_DEFAULT_SERVER_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%s'",
            JBHOOK1_CONFIG_EAMUSE_SERVER_KEY,
            JBHOOK1_CONFIG_EAMUSE_DEFAULT_SERVER_VALUE);
    }

    if (!net_str_parse(server_url, &config_eamuse->server)) {
        memcpy(
            &config_eamuse->server,
            &jbhook1_eamuse_default_server,
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
            JBHOOK1_CONFIG_EAMUSE_PCBID_KEY,
            (uint8_t *) &config_eamuse->pcbid,
            JBHOOK1_CONFIG_EAMUSE_DEFAULT_PCBID_VALUE_LEN,
            (uint8_t *) &JBHOOK1_CONFIG_EAMUSE_DEFAULT_PCBID_VALUE)) {
        tmp = security_id_to_str(
            &JBHOOK1_CONFIG_EAMUSE_DEFAULT_PCBID_VALUE, false);
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default",
            tmp);
        free(tmp);
    }

    if (!security_id_verify(&config_eamuse->pcbid)) {
        log_fatal("PCBID verification failed");
        return;
    }

    if (!cconfig_util_get_data(
            config,
            JBHOOK1_CONFIG_EAMUSE_EAMID_KEY,
            (uint8_t *) &config_eamuse->eamid,
            JBHOOK1_CONFIG_EAMUSE_DEFAULT_EAMID_VALUE_LEN,
            (uint8_t *) &JBHOOK1_CONFIG_EAMUSE_DEFAULT_EAMID_VALUE)) {
        tmp = security_id_to_str(
            &JBHOOK1_CONFIG_EAMUSE_DEFAULT_EAMID_VALUE, false);
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default",
            tmp);
        free(tmp);
    }

    if (!security_id_verify(&config_eamuse->eamid)) {
        log_fatal("EAMID verification failed");
        return;
    }
}
