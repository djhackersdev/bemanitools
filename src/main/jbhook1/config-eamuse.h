#ifndef JBHOOK1_CONFIG_EAMUSE_H
#define JBHOOK1_CONFIG_EAMUSE_H

#include "cconfig/cconfig.h"

#include "security/id.h"

#include "util/net.h"

/**
 * Struct holding configuration values for eamuse related items.
 */
struct jbhook1_config_eamuse {
    struct net_addr server;
    struct security_id pcbid;
    struct security_id eamid;
};

/**
 * Initialize a cconfig structure with the basic structure and default values
 * of this configuration.
 */
void jbhook1_config_eamuse_init(struct cconfig *config);

/**
 * Read the module specific config struct values from the provided cconfig
 * struct.
 *
 * @param config_eamuse Target module specific struct to read configuration
 *                      values to.
 * @param config cconfig struct holding the intermediate data to read from.
 */
void jbhook1_config_eamuse_get(
    struct jbhook1_config_eamuse *config_eamuse, struct cconfig *config);

#endif