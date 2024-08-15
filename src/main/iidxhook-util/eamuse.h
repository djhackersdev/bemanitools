#ifndef IIDXHOOK_EAMUSE_H
#define IIDXHOOK_EAMUSE_H

#include "util/net.h"

/**
 * Hook various calls resolving the service address to connect to
 * the eamuse server for the old IIDX games (9th to DistorteD)
 */
void eamuse_hook_init(void);

void eamuse_hook_fini();

/**
 * Set a net_addr to a eamuse server for the game to connect to.
 *
 * @param addr net_addr struct with address to the server.
 */
void eamuse_set_addr(const struct net_addr *addr);

/**
 * Check if the target eamuse server is reachable. Reports success/error using
 * the logger.
 */
void eamuse_check_connection();

#endif
