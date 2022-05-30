#ifndef DDRHOOK1_CONFIG_SECURITY_H
#define DDRHOOK1_CONFIG_SECURITY_H

#include "cconfig/cconfig.h"

#include "security/mcode.h"

#include "util/net.h"

/**
 * Struct holding configuration values for security related items.
 */
struct ddrhook1_config_security {
    struct security_mcode mcode;
};

/**
 * Initialize a cconfig structure with the basic structure and default values
 * of this configuration.
 */
void ddrhook1_config_security_init(struct cconfig *config);

/**
 * Read the module specific config struct values from the provided cconfig
 * struct.
 *
 * @param config_security Target module specific struct to read configuration
 *                        values to.
 * @param config cconfig struct holding the intermediate data to read from.
 */
void ddrhook1_config_security_get(
    struct ddrhook1_config_security *config_security, struct cconfig *config);

#endif